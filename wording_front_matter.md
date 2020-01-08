% A Unified Executors Proposal for C++ | P0443R12

----------------    -------------------------------------
Title:              A Unified Executors Proposal for C++

Authors:            Jared Hoberock, jhoberock@nvidia.com

                    Michael Garland, mgarland@nvidia.com

                    Chris Kohlhoff, chris@kohlhoff.com

                    Chris Mysen, mysen@google.com

                    Carter Edwards, hcedwar@sandia.gov

                    Gordon Brown, gordon@codeplay.com

                    David Hollman, dshollm@sandia.gov

                    Lee Howes, lwh@fb.com

                    Kirk Shoop, kirkshoop@fb.com

                    Lewis Baker, lbaker@fb.com

                    Eric Niebler, eniebler@fb.com

Other Contributors: Hans Boehm, hboehm@google.com

                    Thomas Heller, thom.heller@gmail.com

                    Bryce Lelbach, brycelelbach@gmail.com

                    Hartmut Kaiser, hartmut.kaiser@gmail.com

                    Bryce Lelbach, brycelelbach@gmail.com

                    Gor Nishanov, gorn@microsoft.com

                    Thomas Rodgers, rodgert@twrodgers.com

                    Michael Wong, michael@codeplay.com

Document Number:    P0443R12

Date:               2020-01-13

Audience:           SG1 - Concurrency and Parallelism, LEWG

Reply-to:           sg1-exec@googlegroups.com

Abstract:           This paper proposes [a programming model](#proposed-wording) for executors, which are modular components for creating execution, and senders, which are lazy descriptions of execution.

------------------------------------------------------

# Design Document

## Motivation

When we imagine the future of C++ programs, we envision elegant compositions of
networked asynchronous parallel computations accelerated by diverse hardware
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
collaboration](#appendix-a-executors-bibilography), SG1 adopted this design by
universal consensus at the Cologne meeting in 2019.

## Usage Example

[P0443](https://wg21.link/P0443) defines requirements for two key components of
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

// alternatively, use low-level P0443 APIs directly

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

Executors provide a uniform interface for work creation by abstracting
underlying resources where work physically executes. The previous code
example's underlying resource was a thread pool. Other examples include SIMD
units, GPU runtimes, or simply the current thread. In general, we call such
resources **execution contexts**. The purpose of executors is to be uniform,
lightweight handles to execution contexts which may be inefficient,
inconvenient, or impossible to access directly. Uniformity enables
programmer control over where work executes, even work created
indirectly behind library interfaces.

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
As a consequence, we expect most programmers to interact with executors
indirectly via higher-level libraries which manipulate executors directly on
behalf of their client. Our goal of an asynchronous STL being an example of
such a library.

For example, consider how `std::async` could be extended to interoperate
with executors enabling client control over execution:

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

The executor is the conduit through which clients exercise precise control over
where the library executes work. For example, a client can select from among
multiple thread pools to control exactly which pool is used simply by providing
a corresponding executor. Inconveniences of work packaging and submission
becomes the library's responsibility.

**Authoring executors.** Programmers author custom executor types by defining a
type with an `execute` function. Consider an implementation of an executor
whose `execute` function executes the client's work "inline" on the calling
thread:

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

**Customizing executor operations.** Programmers inevitably require accelerated
or customized execution. The previous example demonstrated custom execution at
the granularity of a new executor type. However, finer-grained and
coarser-grained customization techniques are also possible. These are
**executor properties** and **control structures**, respectively.

**Executor properties** allow communication between executors and clients
regarding optional guarantees beyond the contract of `execute`. In principle,
optional, dynamic data members or function parameters could act as
communication channels, but we require the ability to introduce
customization at compile time. Moreover, optional parameters lead to
a combinatorial number of function variants.

Instead, statically-actionable properties factor such guarantees and thereby
avoid a combinatorial explosion of executor APIs. For example, consider the
desire to execute blocking work with priority. An unscalable design might
embed these behaviors into the `execute` interface by multiplying the
individual factors into separate functions: `execute`, `blocking_execute`,
`execute_with_priority`, `blocking_execute_with_priority`, etc.
[P0443](https://wg21.link/P0443) identifies several such guarantees.

[P0443](https://wg21.link/P0443) avoids this unscalable situation by adopting [P1393](https://wg21.link/P1393)'s properties design based on `require` and `prefer`:

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
weaker request used to communicate hints.

Interfaces may constrain executors to ensure the properties they need are
`require`able. Consider a version of `std::async` which *never* blocks the
caller:

```P0443
template<executor E, class F, class... Args>
  requires can_require_v<E, execution::blocking_t.always_t>
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

**Control structures** allow customizations at a higher level of abstraction by
allowing executors to "hook" them. Customization is useful when a particular
execution context offers an efficient implementation. The first such control
structure defined by [P0443](https://wg21.link/P0443) is `bulk_execute`, which
creates a group of function invocations in a single operation. This pattern
permits a wide range of efficient implementations and is of fundamental
importance to C++ programs and the standard library.

By default, `bulk_execute` invokes `execute` repeatedly. However, repeatedly
executing individual work items is inefficient at scale. Consequently, many
platforms provide APIs that explicitly and efficiently execute bulk work. In
such cases, a custom `bulk_execute` avoids inefficient platform interactions
via direct access to these accelerated APIs.

`bulk_execute` receives an invocable and an invocation count. Consider a possible SIMD implementation:

```P0443
struct simd_executor : inline_executor {
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

First, `simd_executor` satisfies the `executor` requirements by inheriting our `inline_executor` example.
To accelerate `bulk_execute`, it uses a SIMD loop.

`bulk_execute` should be used in cases where multiple pieces of work are available at once:

```P0443
template<class Executor, class F, class Range>
void my_for_each(const Executor& ex, F f, Range rng) {
  // request bulk execution, receive a sender
  sender auto s = execution::bulk_execute(ex, [=](size_t i) {
    f(rng[i]);
  });

  // initiate execution and wait for it to complete
  execution::sync_wait(s);
}
```

Our example `bulk_execute` implementation executes "eagerly" and blocks its
caller.  As `my_for_each` demonstrates, unlike `execute`, `bulk_execute` is an
example of a "lazy" operation whose execution may be optionally postponed. The
token this `bulk_execute` returns is an example of a sender a client may use to
initiate execution. Lazy execution via senders and receivers is the subject of
the next section.

## Senders and Receivers Represent Work

The `executor` concept addresses a basic need in a simple way: the need to
launch a single operation in a specified execution context. The expressive
power of `executor` is rather limited, however: since `execute` returns `void`
instead of a handle to the just-scheduled work, the `executor` abstraction
gives no generic way to chain operations and consequently no way to propagate
values, errors, and cancellation signals down chains of dependent operations;
no way to specify on a per-operation basis how to handle scheduling errors
(errors internal to an executor that happen between when work is enqueued and
 when it is run); and no way to control the allocation and lifetime of the
state associated with an operation short of writing custom allocators.

Without a work scheduling abstraction that offers such controls, it is not
possible to define Generic (in the Stepanov sense) asynchronous algorithms that
compose, and do so efficiently and with sensible default implementations. To
fill this gap, this paper proposes two related abstractions, `sender` and
`receiver`, which will be described later.

### Generic async algorithm example: `retry`

An example of the kind of Generic async algorithm sender/receiver hopes to
enable is `retry`. `retry` has very simple semantics: Schedule work on an
execution context; if the execution succeeds, done; otherwise, if the user
requests cancellation, done; otherwise, when a scheduling error occurs, retry
the operation.

```P0443
template<invocable Fn>
void retry( executor_of<Fn> auto ex, Fn fn ) {
  // ???
}
```

With the executor abstraction, this algorithm is impossible to implement
generically because there is no portable way to intercept and react to
scheduling errors. Later we show how this algorithm might look when implemented
with `sender`/`receiver`.

### Goal: An Asynchronous STL

With suitably chosen concepts and a set of Generic async algorithms like
`retry` defined in terms of them, it should be possible to easily create DAGs
of async computation that span multiple execution contexts and efficiently
execute them. Here is some sample syntax for the sorts of async programs we
would like to enable (borrowed from [P1897](http://wg21.link/P1897)):

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
whose return type satisfies the correct concept and have this program continue
to work. Generic algorithms like `when_all` and `when_any` would permit users
to express fork/join concurrency in their DAGs. As with STL’s `iterator`
abstraction, the cost of satisfying the conceptual requirements are offset by
the expressive wins of being able to reuse a large and composable library of
algorithms.

### Current techniques

There are many extant techniques for creating chains of dependent asynchronous
execution. Ordinary callbacks have been used very successfully for many years
to create asynchronous dependent chains of dependent work in C++ and elsewhere.
More modern codebases have switched to using some variation of the
promise/future abstraction that support continuations (e.g.,
    `std::experimental::future::then`).  In C++20 and beyond, we could even
imagine standardizing on coroutines, so that launching an async operation
returns an awaitable. Each of these approaches has strengths and weaknesses.

**Promises and futures**, as traditionally realized, require the dynamic
allocation and management of a shared control block, some form of
synchronization, and typically type-erasure of both the work and the
continuation. Many of these costs are inherent in the nature of “future” as a
handle to an operation that is already scheduled for execution. These expenses
rule out the promise/future abstraction for many uses and makes it a poor
choice for a basis of a Generic mechanism.

**Coroutines** have many of the same problems as promise/future except that, by
typically starting suspended, they avoid the need for synchronization when
chaining dependent work. In many cases the coroutine frame is dynamically
allocated, and the allocation is not easily avoided. Using coroutines in
embedded or heterogeneous environments would require great attention to detail.
There is also no accommodation made for cancellation. Two coroutines, one
yielding values from a loop and the other consuming them in a loop, cannot
safely coordinate early loop termination without either using exceptions —
which are inefficient and disallowed in many environments — or by an *ad hoc*
mechanism whereby `co_yield` returns a status code, which consequently needs to
be checked and acted upon explicitly and consistently to ensure correctness.
(See [P1662](http://wg21.link/P1662) for a complete discussion.)

**Callbacks** are the simplest, most powerful, and most efficient mechanism for
creating chains of work, but they are not without their own problems. Errors
happen, so there is a need to propagate either an error or a value. This simple
requirement has led to many different callback shapes. The lack of a standard
callback signature stands as an obstacle to Generic design. Additionally, few
if any of these shapes accommodate the need to also propagate a cancellation
signal when the user has requested upstream work to stop and clean up.

## Receiver, sender, and scheduler

With the preceding as motivation, we introduce the sender/receiver abstraction
as way to address the needs of Generic asynchronous programming in the presence
of value, error, and cancellation propagation.

### Receiver

A `receiver` is simply a callback with a particular shape and semantics. Unlike
a traditional callback with uses function-call syntax and a single signature
that handles both success and error cases, a receiver has three separate
channels for value, error, and “done” (aka cancelled). These are specified as
customization points: `set_value`, `set_error`, and `set_done` in the
`std::execution` namespace.

A type `R` that satisfies the `receiver_of<R, Ts...>` concept supports the following syntax:

```P0443
std::execution::set_value(r, ts...); // success
std::execution::set_error(r, ep);    // error (ep is std::exception_ptr)
std::execution::set_done(r);         // stopped
```

`set_error` and `set_done` are guaranteed no-fail (`noexcept`). Exactly one of
the three functions must be called on a `receiver` before it is destroyed. Each
of these interfaces is considered “terminal”; that is, a particular receiver
may assume that if one is called, that no others will ever be — the exception
being that if `set_value` exits with an exception, the receiver has not yet
completed, and another function must be called on it before it is destroyed.
After a failed call to `set_value`, a subsequent call to either `set_error` or
`set_done` is required to be valid; a receiver need not guarantee that a second
call to `set_value` is well-formed. Collectively, these semantic constraints
are known as the “*receiver contract*”.

Although the shape of `receiver` appears novel at first glance, it remains in
essence just a callback. Its shape and semantics are chosen because, together
with `sender`, it facilitates the Generic implementation of many useful async
algorithms like `retry`, as will be shown. And its novelty dissolves when you
consider that a `std::promise` has essentially the same shape, and it is used
as a special kind of callback although we tend not to think of it as such.

### Sender

Conceptually, a `sender` is work that has not been scheduled for execution
yet, to which one may add a continuation (a `receiver`) and then “launch”, or
enqueue for execution.

In this revision of this paper (R12), these two operations — attach a
continuation and launch — are combined into a single `void`-returning basis
operation called `submit`, which is a customization point that lives in the
`std::execution` namespace. [P2006](http://wg21.link/2006) proposes separating
these two operations into a `connect` operation that joins a `sender` and a
`receiver`, returning an operation state; and a `start` operation that enqueues
the operation state for execution.

```P0443
// P0443R12
std::execution::submit(snd, rec);

// P0443R12 + P2006R0
auto op { std::execution::connect(snd, rec) };
// ... later
std::execution::start(op);
```

Given `connect` and `start`, which are necessarily primitive and low-level, it
is possible to implement `submit` generically. The operation state returned
from `connect` must have a lifetime that spans the duration of the async
operation. Additionally, it must have an address that is stable. In truly
asynchronous scenarios, it is typically a member of a dynamically allocated
object, although in a coroutine the state will typically be a local variable
stored in a coroutine frame without the need for a separate allocation.

A sender is said to “complete” when it satisfies the *receiver contract* of the
receiver to which it is attached; that is, when it calls one of the three
`receiver` functions and that function returns non-exceptionally. When a sender
in its implementation contains a call to `set_value(r, ts...)`, we say it is a
“sender of `Ts...`”. If it contains a call to `set_value(r)`, we say it is a
“sender of `void`”.

The `sender` concept itself places no requirements on the execution context on
which a sender completes. Rather, specific models of the `sender` concept may
offer stronger guarantees about the context from which the receiver’s methods
will be invoked. This is particularly true of the senders returned from a
`scheduler`’s `schedule` function (described below).

In the general case, senders are “single-shot”; that is, `connect` can be
called on them at most once. Some senders can be be `connect`-ed many times.
Generally, if `connect` on an lvalue sender is well-formed, it is safe to
`connect` multiple receivers to it.

### Scheduler

Many generic async algorithms need to create multiple execution agents on the
same execution context. Therefore, it is insufficient to parameterize these
algorithms with a single-shot sender that completes in a known context. Rather,
it makes sense to pass to these algorithms a factory of such
single-shot senders. In the design presented in this document, the
factory is called “`scheduler`”, and it has a single basis
operation: `schedule`.

Given an object `sched` whose type satisfies `scheduler`, the following
expression is well-formed:

```P0443
sender auto s = std::execution::schedule(sched);
// OK, s is a single-shot sender of void that completes in sched's execution context
```

We can imagine subsumptions of the `scheduler` concept that add the ability to
schedule at a particular time (`schedule_at`), or after a particular duration
(`schedule_after`), or a deadline scheduler that drops work if it is not
executed within a time window (`schedule_before`), or one that schedules with a
given priority (`schedule_pri`). All of these would return `sender`s of `void`.

## Generic algorithms and sender/receiver

Concepts are discovered through iterative refinement, generalization, and
refactoring of a common and useful set of algorithms within a particular
domain. The acid test, then, of any set of concepts is how well they can be
used to constrain such algorithms, and how efficiently the algorithms can be
implemented in terms of the concepts’ basis operations. Such an iterative
process led to `sender`, `receiver`, and `scheduler`. Below, we show how these
concepts are usable to provide efficient default implementations of several
common async algorithms.

Most generic async algorithms are implemented as taking a sender and returning
a sender whose `connect` method wraps the passed-in receiver in a custom
receiver that implements the algorithm’s logic. The `then` algorithm below,
which chains a continuation function on a `sender`, is a simple
demonstration of this.

### Algorithm `then`

The following code implements a `then` algorithm that, like
`std::experimental::future::then`, schedules a transformation function to be
applied to the result of an asynchronous operation when it becomes available.
This code demonstrates how an algorithm can use receiver adaptors that codify
the algorithm’s logic.

```P0443
template<receiver R, class F>
struct _then_receiver {
    R r_;
    F f_;

    // Handle the case when the callable returns non-void:
    // Not shown: handle the case when the callable returns void
    template<class... As>
      requires receiver_of<R, invoke_result_t<F, As...>>
    void set_value(Args&&... args) && noexcept(/*...*/) {
        ::set_value((R&&) r_, invoke((F&&) f_, (As&&) as...));
    }

    template<class E>
      requires receiver<R, E>
    void set_error(E&& e) && noexcept {
        ::set_error((R&&) r_, (E&&) e);
    }

    void set_done() && noexcept {
        ::set_done((R&&) r_);
    }
};

template<sender S, class F>
struct _then_sender : sender_base {
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

Given some asynchronous, `sender`-returning API `async_foo`, a user may use
`then` to execute some code after the async result becomes available:

```P0443
sender auto s = then( async_foo(args...), [](auto result) { stuff...} );
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

In the `sender`/`receiver` design, `retry` is an algorithm on multi-shot
senders. As described above, the idea of `retry` is to retry the async
operation on failure, but not on success or cancellation. Key to a correct
generic implementation of `retry` is the ability to distinguish the error case
from the cancelled case.

As with the `then` algorithm, the `retry` algorithm places the logic of the
algorithm into a custom receiver to which the sender to be retried is
`connect`-ed. That custom receiver has `set_value` and `set_done` members that
simply pass their signals through unmodified. The `set_error` member, on the
other hand, reconstructs the operation state in-place by making another call to
`connect` with the original sender and a new instance of the custom receiver.
That new operation state is then `start`-ed again, which effectively causes the
original sender to be retried.

[The appendix](#appendix-b-the-retry-algorithm) has the source of the `retry` algorithm. Note that the signature of
the retry algorithm is simply:

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

### Summary

The algorithms `then` and `retry` are only two of scores of interesting Generic
asynchronous algorithms that are expressible in terms of sender/receiver. Two
other important algorithms are `on` and `via`, the former which schedules a
sender for execution on a particular `scheduler`, and the latter which causes a
sender’s *continuations* to be run on a particular `scheduler`. In this way,
chains of asynchronous computation can be created that transition from one
execution context to another.

Other important algorithms are `when_all` and `when_any`, which encapsulate
different fork/join semantics. With these algorithms and the others, entire
DAGs of async computation can be created and executed. `when_any` can in turn
be used to implement a generic `timeout` algorithm, together with a sender that
sleeps for a duration and then sends a “done” signal, and so these algorithms
compose.

In short, sender/receiver permits a rich set of Generic asynchronous algorithms
to sit alongside Stepanov’s sequence algorithms in the STL. Asynchronous APIs
that return senders would be usable with these Generic algorithms, increasing
reusability. [P1897](http://wg21.link/P1897) suggest an initial set of these
algorithms. (*Note:* in P1897, the `then` algorithm is called `transform`.)

### A note on coroutines

As a final note, we would like to refer to [P1341](http://wg21.link/P1341),
which leverages the structural similarities between coroutines and the
sender/receiver abstraction to give a class of senders a standard-provided
`operator co_await`. The end result is that a sender, simply by dint of being a
sender, can be `co_await`-ed in a coroutine. With the refinement of
sender/receiver being proposed in P2006 — namely, the splitting of `submit`
into `connect`/`start` — that automatic adaptation from sender-to-awaitable is
allocation- and synchronization-free.

## Summary

We envision a future when C++ programmers can express parallel execution of
work on diverse hardware resources through elegant and standard interfaces.
P0443 provides an initial step towards that goal by specifying a foundation for
flexible execution. User-customizable **executors** represent hardware
resources that execute work. **Senders and receivers** represent
lazily-constructed chains of work. These tools allow clients to control how,
  where, and when work happens.

# Proposed Wording

