# Design Document

## Motivation

When we imagine the future of C++ programs, we envision elegant compositions of
networked, asynchronous parallel computations accelerated by diverse hardware,
ranging from tiny mobile devices to giant supercomputers. In the present, 
hardware diversity is greater than ever, but C++ programmers lack
satisfying parallel programming tools for them. Industrial-strength
concurrency primitives like `std::thread` and `std::atomic` are powerful but
hazardous. `std::async` and `std::future` suffer from well-known problems. And
the standard algorithms library, though parallelized, remains inflexible and
non-composable.

To address these temporary challenges and build toward the future, C++ must lay
a foundation for controlling program execution. First, **C++ must provide
flexible facilities to control where and when work happens.** This paper
proposes a design for those facilities. After [much discussion and
collaboration](#appendix-executors-bibilography), SG1 adopted this design by
universal consensus at the Cologne meeting in 2019.

## Usage Example

This proposal defines requirements for two key components of
execution: a work execution interface and a representation of work and
their interrelationships. Respectively, these are **executors** and **senders
and receivers**:

```P0443
// make P0443 APIs in namespace std::execution available
using namespace std::execution;

// get an executor from somewhere, e.g. a thread pool
std::static_thread_pool pool(16);
executor auto ex = pool.executor();

// use the executor to describe where some high-level library should execute its work
perform_business_logic(ex);

// alternatively, use primitive P0443 APIs directly

// immediately submit work to the pool
execute(ex, []{ std::cout << "Hello world from the thread pool!"; });

// immediately submit work to the pool and require this thread to block until completion
execute(std::require(ex, blocking.always), foo);

// describe a chain of dependent work to submit later
sender auto begin    = schedule(ex);
sender auto hi_again = then(begin, []{ std::cout << "Hi again! Have an int."; return 13; });
sender auto work     = then(hi_again, [](int arg) { return arg + 42; });

// prints the final result
receiver auto print_result = as_receiver([](int arg) { std::cout << "Received " << std::endl; });

// submit the work for execution on the pool by combining with the receiver 
submit(work, print_result);

// Blue: proposed by P0443. Teal: possible extensions.
```

## Executors Execute Work

As lightweight handles, executors impose uniform access to execution contexts.

Executors provide a uniform interface for work creation by abstracting
underlying resources where work physically executes. The previous code
example's underlying resource was a thread pool. Other examples include SIMD
units, GPU runtimes, or simply the current thread. In general, we call such
resources **execution contexts**. As lightweight handles, executors impose
uniform access to execution contexts. Uniformity enables control over where
work executes, even when it is executed indirectly behind library interfaces.

The basic executor interface is the `execute` function through which clients
execute work: 

```P0443
// obtain an executor
executor auto ex = ...

// define our work as a nullary invocable
invocable auto work = []{ cout << "My work" << endl; };

// execute our work via the execute customization point
execute(ex, work);
```

On its own, `execute` is a primitive "fire-and-forget"-style interface. It
accepts a single nullary invocable, and returns nothing to identify or interact
with the work it creates. In this way, it trades convenience for universality.
As a consequence, we expect most programmers to interact with executors via
more convenient higher-level libraries, our envisioned asynchronous STL being such
an example.

Consider how `std::async` could be extended to interoperate with executors
enabling client control over execution:

```P0443
template<class Executor, class F, class Args...>
future<invoke_result_t<F,Args...>> async(const Executor& ex, F&& f, Args&&... args) {
  // package up the work
  packaged_task work(forward<F>(f), forward<Args>(args)...);

  // get the future
  auto result = work.get_future();

  // execute work on the given executor
  execution::execute(ex, move(work));

  return result;
}
```

The benefit of such an extension is that a client can select from among
multiple thread pools to control exactly which pool `std::async` uses simply by
providing a corresponding executor. Inconveniences of work packaging and
submission become the library's responsibility.

**Authoring executors.** Programmers author custom executor types by defining a
type with an `execute` function. Consider the implementation of an executor
whose `execute` function executes the client's work "inline":

```P0443
struct inline_executor {
  // define execute
  template<class F>
  void execute(F&& f) const noexcept {
    std::invoke(std::forward<F>(f));
  }

  // enable comparisons
  auto operator<=>(const inline_executor&) const = default;
};
```

Additionally, a comparison function determines whether two executor objects
refer to the same underlying resource and therefore execute with equivalent
semantics. Concepts `executor` and `executor_of` summarize these requirements.
The former validates executors in isolation; the latter, when both executor and
work are available.

**Executor customization** can accelerate execution or introduce novel
behavior. The previous example demonstrated custom execution at the
granularity of a new executor type, but finer-grained and coarser-grained
customization techniques are also possible. These are **executor properties**
and **control structures**, respectively.

**Executor properties** communicate optional behavioral requirements beyond the
minimal contract of `execute`, and this proposal specifies several. We expect
expert implementors to impose these requirements beneath higher-level
abstractions. In principle, optional, dynamic data members or function
parameters could communicate these requirements, but C++ requires the ability
to introduce customization at compile time. Moreover, optional parameters lead
to [combinatorially many function variants](https://wg21.link/P2033).

Instead, statically-actionable properties factor such requirements and thereby
avoid a combinatorial explosion of executor APIs. For example, consider the
requirement to execute blocking work with priority. An unscalable design might
embed these options into the `execute` interface by multiplying individual
factors into separate functions: `execute`, `blocking_execute`,
`execute_with_priority`, `blocking_execute_with_priority`, etc.

Executors avoid this unscalable situation by adopting
[P1393](https://wg21.link/P1393)'s properties design based on `require` and
`prefer`:

```P0443
// obtain an executor
executor auto ex = ...;

// require the execute operation to block
executor auto blocking_ex = std::require(ex, execution::blocking.always);

// prefer to execute with a particular priority p
executor auto blocking_ex_with_priority = std::prefer(blocking_ex, execution::priority(p));

// execute my blocking, possibly prioritized work
execution::execute(blocking_ex_with_priority, work);
```

Each application of `require` or `prefer` transforms an executor into one with
the requested property. In this example, if `ex` cannot be transformed into a
blocking executor, the call to `require` will fail to compile. `prefer` is a
weaker request used to communicate hints and consequently always succeeds
because it may ignore the request.

Consider a version of `std::async` which *never* blocks the caller:

```P0443
template<executor E, class F, class... Args>
auto really_async(const E& ex, F&& f, Args&&... args) {
  using namespace execution;

  // package up the work
  packaged_task work(forward<F>(f), forward<Args>(args)...);

  // get the future
  auto result = work.get_future();

  // execute the nonblocking work on the given executor
  execute(require(ex, blocking.never), move(work));

  return result;
}
```

Such an enhancement could address a well-known hazard of `std::async`:

```P0443
// confusingly, always blocks in the returned but discarded future's destructor
std::async(foo);

// *never* blocks
really_async(foo);
```

**Control structures** permit customizations at a higher level of abstraction
by allowing executors to "hook" them and is useful when an efficient
implementation is possible on a particular execution context. The first such
control structure this proposal defines is `bulk_execute`, which creates a
group of function invocations in a single operation. This pattern permits a
wide range of efficient implementations and is of fundamental importance to C++
programs and the standard library.

By default, `bulk_execute` invokes `execute` repeatedly, but repeatedly
executing individual work items is inefficient at scale. Consequently, many
platforms provide APIs that explicitly and efficiently execute bulk work. In
such cases, a custom `bulk_execute` avoids inefficient platform interactions
via direct access to these accelerated bulk APIs while also optimizing the use
of scalar APIs.

`bulk_execute` receives an invocable and an invocation count. Consider a possible implementation:

```P0443
struct simd_executor : inline_executor { // first, satisfy executor requirements via inheritance
  template<class F>
  simd_sender bulk_execute(F f, size_t n) const {
    #pragma simd
    for(size_t i = 0; i != n; ++i) {
      std::invoke(f, i);
    }

    return {};
  }
};
```

To accelerate `bulk_execute`, `simd_executor` uses a SIMD loop.

`bulk_execute` should be used in cases where multiple pieces of work are available at once:

```P0443
template<class Executor, class F, class Range>
void my_for_each(const Executor& ex, F f, Range rng) {
  // request bulk execution, receive a sender
  sender auto s = execution::bulk_execute(ex, [=](size_t i) {
    f(rng[i]);
  }, std::ranges::size(rng));

  // initiate execution and wait for it to complete
  execution::sync_wait(s);
}
```

`simd_executor`'s particular `bulk_execute` implementation executes "eagerly",
but `bulk_execute`'s semantics do not require it. As `my_for_each` demonstrates, unlike
`execute`, `bulk_execute` is an example of a "lazy" operation whose execution
may be optionally postponed. The token this `bulk_execute` returns is an
example of a sender a client may use to initiate execution or otherwise
interact with the work. For example, calling `sync_wait` on the sender
ensures that the bulk work completes before the caller continues. Senders
and receivers are the subject of the next section.

## Senders and Receivers Represent Work

The `executor` concept addresses a basic need of executing a single operation
in a specified execution context. The expressive power of `executor` is
limited, however: since `execute` returns `void` instead of a handle to the
just-scheduled work, the `executor` abstraction gives no generic way to chain
operations and thereby propagate values, errors, and cancellation signals
downstream; no way to handle scheduling errors occurring between when work
submission and execution; and no convenient way to control the allocation and
lifetime of state associated with an operation.

Without such controls, it is not possible to define Generic (in the Stepanov
sense) asynchronous algorithms that compose efficiently with sensible
default implementations. To fill this gap, this paper proposes two related
abstractions, `sender` and `receiver`, concretely motivated below.

### Generic async algorithm example: `retry`

`retry` is the kind of Generic algorithm senders and receivers enable.
It has simple semantics: schedule work on an execution context; if the
execution succeeds, done; otherwise, if the user requests cancellation, done;
otherwise, if a scheduling error occurs, try again.

```P0443
template<invocable Fn>
void retry(executor_of<Fn> auto ex, Fn fn) {
  // ???
}
```

Executors alone prohibit a generic implementation because they lack a portable
way to intercept and react to scheduling errors. Later we show how this
algorithm might look when implemented with senders and receivers.

### Goal: an asynchronous STL

Suitably chosen concepts driving the definition of Generic async algorithms
like `retry` streamline the creation of efficient, asynchronous graphs of work.
Here is some sample syntax for the sorts of async programs we envision
(borrowed from [P1897](http://wg21.link/P1897)):

```P0443
sender auto s = just(3) |                                  // produce '3' immediately
                via(scheduler1) |                          // transition context
                then([](int a){return a+1;}) |             // chain continuation
                then([](int a){return a*2;}) |             // chain another continuation
                via(scheduler2) |                          // transition context
                handle_error([](auto e){return just(3);}); // with default value on errors
int r = sync_wait(s);                                      // wait for the result
```

It should be possible to replace `just(3)` with a call to any asynchronous API
whose return type satisfies the correct concept and maintain this program's
correctness. Generic algorithms like `when_all` and `when_any` would permit
users to express fork/join concurrency in their DAGs. As with STL’s `iterator`
abstraction, the cost of satisfying the conceptual requirements are offset by
the expressivity of a large reusable and composable library of algorithms.

### Current techniques

There are many techniques for creating chains of dependent asynchronous
execution. Ordinary callbacks have enjoyed success in C++ and elsewhere for
years. Modern codebases have switched to variations of future abstractions
that support continuations (e.g., `std::experimental::future::then`). In C++20
and beyond, we could imagine standardizing on coroutines, so that launching an
async operation returns an awaitable. Each of these approaches has strengths
and weaknesses.

**Futures**, as traditionally realized, require the dynamic
allocation and management of a shared state, synchronization, and typically
type-erasure of work and continuation. Many of these costs are inherent in the
nature of “future” as a handle to an operation that is already scheduled for
execution. These expenses rule out the future abstraction for many uses
and makes it a poor choice for a basis of a Generic mechanism.

**Coroutines** suffer many of the same problems but can avoid synchronizing
when chaining dependent work because they typically start suspended. In many
cases, coroutine frames require unavoidable dynamic allocation. Consequently,
coroutines in embedded or heterogeneous environments require great attention
to detail. Neither are coroutines good candidates for cancellation because the early and safe termination
of coordinating coroutines requires unsatisfying solutions. On the one hand,
exceptions are inefficient and disallowed in many environments.
Alternatively, clumsy *ad hoc* mechanisms, whereby `co_yield` returns a status code,
hinder correctness. [P1662](http://wg21.link/P1662)
provides a complete discussion.

**Callbacks** are the simplest, most powerful, and most efficient mechanism for
creating chains of work, but suffer problems of their own. Callbacks must
propagate either errors or values. This simple requirement yields many
different interface possibilities, but the lack of a standard obstructs Generic
design. Additionally, few of these possibilities accomodate cancellation
signals when the user requests upstream work to stop and clean up.

## Receiver, sender, and scheduler

With the preceding as motivation, we introduce primitives to address the needs of Generic asynchronous programming in the
presence of value, error, and cancellation propagation.

### Receiver

A `receiver` is simply a callback with a particular interface and semantics. Unlike
a traditional callback which uses function-call syntax and a single signature
handling both success and error cases, a receiver has three separate
channels for value, error, and “done” (aka cancelled).

These channels are specified as customization points, and a type `R` modeling `receiver_of<R,Ts...>` supports them:

```P0443
std::execution::set_value(r, ts...); // signal success, but set_value itself may fail
std::execution::set_error(r, ep);    // signal error (ep is std::exception_ptr), never fails
std::execution::set_done(r);         // signal stopped, never fails
```

Exactly one of the three functions must be called on a `receiver` before it is
destroyed. Each of these interfaces is considered “terminal”. That is, a
particular receiver may assume that if one is called, no others ever will be.
The one exception being if `set_value` exits with an exception, the receiver is
not yet complete. Consequently, another function must be called on it before it
is destroyed. After a failed call to `set_value`, correctness requires a
subsequent call either to `set_error` or `set_done`; a receiver need not
guarantee that a second call to `set_value` is well-formed. Collectively, these
requirements are the “*receiver contract*”.

Although `receiver`'s interface appears novel at first glance, it remains just
a callback. Moreover, `receiver`'s novelty disappears when recognizing that
`std::promise`'s `set_value` and `set_exception` provide essentially the same
interface. This choice of interface and semantics, along with `sender`,
facilitate the Generic implementation of many useful async algorithms like
`retry`. 

### Sender

A `sender` represents work that has not been scheduled for execution yet, to
which one must add a continuation (a `receiver`) and then “launch”, or enqueue
for execution. A sender's duty to its connected receiver is to fulfill the
*receiver contract* by ensuring that one of the three `receiver` functions
returns normally.

Earlier versions of this paper fused these two operations — attach a continuation and
launch for execution — into the single operation `submit`. This paper proposes to split
`submit` into a `connect` step that packages a `sender` and a `receiver` into an
operation state, and a `start` step that logically starts the operation and schedules
the receiver completion-signalling methods to be called when the operation completes.

```P0443
// P0443R12
std::execution::submit(snd, rec);

// P0443R13
auto state = std::execution::connect(snd, rec);
// ... later
std::execution::start(state);
```

This split offers interesting opportunities for optimization, and
[harmonizes senders with coroutines](#appendix-a-note-on-coroutines).

The `sender` concept itself places no requirements on the execution context on
which a sender's work executes. Instead, specific models of the `sender` concept may
offer stronger guarantees about the context from which the receiver’s methods
will be invoked. This is particularly true of the senders created by a
`scheduler`.

### Scheduler

Many generic async algorithms create multiple execution agents on the same
execution context. Therefore, it is insufficient to parameterize these
algorithms with a single-shot sender completing in a known context. Rather, it
makes sense to pass these algorithms a factory of single-shot senders. Such
a factory is called a “`scheduler`”, and it has a single basis operation:
`schedule`:

```P0443
sender auto s = std::execution::schedule(sched);
// OK, s is a single-shot sender of void that completes in sched's execution context
```

Like executors, schedulers act as handles to an execution context. Unlike
executors, schedulers submit execution lazily, but a single type may
simultaneously model both concepts. We envision that subsumptions of the
`scheduler` concept will add the ability to postpone or cancel execution until
after some time period has elapsed.

## Senders, receivers, and generic algorithms

Useful concepts constrain generic algorithms while allowing default
implementations via those concepts' basis operations. Below, we show how these
`sender` and `receiver` provide efficient default implementations of common
async algorithms. We envision that most generic async algorithms will be
implemented as taking a sender and returning a sender whose `connect` method
wraps its receiver an adaptor that implements the algorithm’s logic. The `then`
algorithm below, which chains a continuation function on a `sender`, is a
simple demonstration.

### Algorithm `then`

The following code implements a `then` algorithm that, like
`std::experimental::future::then`, schedules a function to be applied to the
result of an asynchronous operation when available. This code demonstrates how
an algorithm can adapt receivers to codify the algorithm’s logic.

```P0443
template<receiver R, class F>
struct _then_receiver : R { // for exposition, inherit set_error and set_done from R
    F f_;

    // Customize set_value by invoking the callable and passing the result to the base class
    template<class... As>
      requires receiver_of<R, invoke_result_t<F, As...>>
    void set_value(Args&&... args) && noexcept(/*...*/) {
        ::set_value((R&&) *this, invoke((F&&) f_, (As&&) as...));
    }

    // Not shown: handle the case when the callable returns void
};

template<sender S, class F>
struct _then_sender : _sender_base {
    S s_;
    F f_;

    template<receiver R>
      requires sender_to<S, _then_receiver<R, F>>
    state_t<S, _then_receiver<R, F>> connect(R r) && {
        return ::connect((S&&)s_, _then_receiver<R, F>{(R&&)r, (F&&)f_});
    }
};

template<sender S, class F>
sender auto then(S s, F f) {
    return _then_sender{{}, (S&&)s, (F&&)f};
}
```

Given some asynchronous, `sender`-returning API `async_foo`, a user of
`then` can execute some code once the async result is available:

```P0443
sender auto s = then(async_foo(args...), [](auto result) {/* stuff... */});
```

This builds a composed asynchronous operation. When the user wants to schedule
this operation for execution, they would `connect` a receiver, and then call
`start` on the resulting operation state.

Scheduling work on an execution context can also be done with `then`. Given a
`static_thread_pool` object `pool` that satisfied the `scheduler` concept, a
user may do the following:

```P0443
sender auto s = then(
    std::execution::schedule( pool ),
    []{ std::printf("hello world"); } );
```

This creates a `sender` that, when submitted, will call `printf` from a thread
in the thread pool.

There exist heterogeneous computing environments that are unable to execute
arbitrary code. For those, an implementation of `then` as shown above would
either not work or would incur the cost of a transition to the host in order to
execute the unknown code. Therefore, `then` itself and several other
fundamental algorithmic primitives, would themselves need to be customizable on
a per-execution context basis.

A full working example of `then` can be found here: [https://godbolt.org/z/dafqM-](https://godbolt.org/z/dafqM-)

### Algorithm `retry`

As described above, the idea of `retry` is to retry the async operation on
failure, but not on success or cancellation. Key to a correct generic
implementation of `retry` is the ability to distinguish the error case from the
cancelled case.

As with the `then` algorithm, the `retry` algorithm places the logic of the
algorithm into a custom receiver to which the sender to be retried is
`connect`-ed. That custom receiver has `set_value` and `set_done` members that
simply pass their signals through unmodified. The `set_error` member, on the
other hand, reconstructs the operation state in-place by making another call to
`connect` with the original sender and a new instance of the custom receiver.
That new operation state is then `start`-ed again, which effectively causes the
original sender to be retried.

[The appendix](#appendix-the-retry-algorithm) lists the source of the `retry` algorithm.
Note that the signature of the retry algorithm is simply:

```P0443
sender auto retry(sender auto s);
```

That is, it is not parameterized on an execution context on which to retry the
operation. That is because we can assume the existence of a function `on` which
schedules a sender for execution on a specified execution context:

```P0443
sender auto on(sender auto s, scheduler auto sched);
```

Given these two functions, a user can simply do `retry(on(s, sched))` to retry
an operation on a particular execution context.

### Toward an asynchronous STL

The algorithms `then` and `retry` are only two of many interesting Generic
asynchronous algorithms that are expressible in terms of senders and receivers. Two
other important algorithms are `on` and `via`, the former which schedules a
sender for execution on a particular `scheduler`, and the latter which causes a
sender’s *continuations* to be run on a particular `scheduler`. In this way,
chains of asynchronous computation can be created that transition from one
execution context to another.

Other important algorithms are `when_all` and `when_any`, encapsulating
fork/join semantics. With these algorithms and others, entire DAGs of async
computation can be created and executed. `when_any` can in turn be used to
implement a generic `timeout` algorithm, together with a sender that sleeps for
a duration and then sends a “done” signal, and so these algorithms compose.

In short, sender/receiver permits a rich set of Generic asynchronous algorithms
to sit alongside Stepanov’s sequence algorithms in the STL. Asynchronous APIs
that return senders would be usable with these Generic algorithms, increasing
reusability. [P1897](http://wg21.link/P1897) suggest an initial set of these
algorithms.

## Summary

We envision a future when C++ programmers can express asynchronous, parallel
execution of work on diverse hardware resources through elegant standard
interfaces. This proposal provides a foundation for flexible exeuction and is
our initial step towards that goal. **Executors** represent hardware resources
that execute work. **Senders and receivers** represent lazily-constructed
asynchronous DAGs of work. These primitives empower programmers to control where and
when work happens.
