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
a foundation for controlling program execution. First, **C++ should provide a
standard way to control how, where, and when work happens.**

## Usage Example

[P0443](https://wg21.link/P0443) defines requirements for two key components of
execution: an execution creation interface and a representation of units of
execution and their interrelationships. Respectively, these are **executors**
and **senders and receivers**:

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

// describe a sequence of dependent tasks to submit later
sender auto begin    = schedule(ex);
sender auto hi_again = then(begin, []{ std::cout << "Hi again! Have an int."; return 13; });
sender auto add_42   = then(hi_again, [](int arg) { return arg + 42; });

// a task to receive the final result, which will simply be discarded
sink_receiver finally;   

// submit the sequence of tasks for execution on the pool by combining with the receiver 
submit(add_42, finally);

// Blue: proposed by P0443. Teal: possible extensions.

```

## Executors Create Work

Executors provide a uniform interface for work creation by abstracting
underlying resources where work physically executes. The previous code
example's underlying resource was a thread pool. Other examples include SIMD
units, GPU runtimes, or simply the current thread. In general, we call such
resources **execution contexts**. The purpose of executors is to be uniform,
lightweight handles to execution contexts which may be inefficient, inconvenient,
or impossible to access directly.

The basic executor interface is the `execute` function through which clients
create units of work: 

```P0443
// obtain an executor
executor auto ex = ...

// define a task as a nullary invocable
invocable auto task = []{ cout << "My task" << endl; };

// execute the task via the execute customization point
execute(ex, task);
```

On its own, `execute` is a primitive "fire-and-forget"-style interface. It
accepts only a single nullary invocable, and does not return anything to
identify or interact with the work it creates. In this way, it trades
convenience for universality. As a consequence, we expect most programmers to
interact with executors indirectly via higher-level libraries which manipulate
executors directly on behalf of their client. Our goal of an asynchronous
STL would be an example of such a library.

For example, consider how `std::async` could be extended to interoperate
with executors enabling client control over execution:

```P0443
template<class Executor, class F, class Args...>
future<invoke_result_t<F,Args...>> async(const Executor& ex, F&& f, Args&&... args) {
  // create a packaged task
  packaged_task task(forward<F>(f), forward<Args>(args)...);

  // get the future
  auto result = task.get_future();

  // execute the task on the given executor
  execution::execute(ex, move(task));

  return result;
}
```

Clients simply provide an appropriate executor, and the library handles inconveniences of task packaging and submission.

**Authoring executors.** Programmers author custom executor types by defining a type with an `execute`
function ^[Alternatively, `execute` may be a free function to retrofit types
unintrusively. `execution::execute` automatically finds the correct
customization.]. Consider a possible implementation of an executor whose
`execute` function executes the client's work "inline" on the calling thread:

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
refer to the same underlying resource and create execution with equivalent
semantics. [P0443](https://wg21.link/P0443) summarizes these requirements in concepts `executor` and
`executor_of`.

**Customizing executor operations.** Programmers will inevitably require accelerated or customized execution. The
previous example demonstrated custom execution at the granularity of a new
executor type. However, finer-grained and coarser-grained customization
techniques are also possible. These are **executor properties** and **control
structures**, respectively.

**Executor properties.** Executor properties allow communication between
executors and clients regarding further guarantees beyond the contract of
`execute`. Properties factor such guarantees and thereby avoid a combinatorial
explosion of executor APIs. For example, consider the desire to execute a
blocking task with priority. An unscalable design might embed these behaviors
into the `execute` interface by multiplying the individual factors into
separate functions: `execute`, `blocking_execute`, `execute_with_priority`,
`blocking_execute_with_priority`, etc.
[P0443](https://wg21.link/P0443) identifies several such guarantees.

[P0443](https://wg21.link/P0443) avoids this unscalable situation by adopting [P1393](https://wg21.link/P1393)'s properties design based on `require` and `prefer`:

```P0443
// obtain an executor
executor auto ex = ...;

// require the execute operation to block
executor auto blocking_ex = std::require(ex, execution::blocking.always);

// prefer to execute with a particular priority p
executor auto blocking_ex_with_priority = std::prefer(blocking_ex, execution::priority(p));

// execute my blocking, possibly prioritized task
execution::execute(blocking_ex_with_priority, task);
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

  // create a packaged task
  packaged_task task(forward<F>(f), forward<Args>(args)...);

  // get the future
  auto result = task.get_future();

  // execute the nonblocking task on the given executor
  execute(require(ex, blocking.never), move(task));

  return result;
}
```

**Control structures.** Executors may also introduce customizations at a higher level of abstraction by
"hooking" algorithmic control structures. Customization is useful when a
particular execution context offers an efficient implementation. The first such
control structure defined by [P0443](https://wg21.link/P0443) is `bulk_execute`, which creates a group of
function invocations in a single operation. By default, `bulk_execute` invokes
`execute` repeatedly. However, many platform APIs provide interfaces that
explicitly create work in bulk. In such cases, a custom `bulk_execute`
avoids inefficient platform interactions via direct access to accelerated APIs.

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

`bulk_execute` should be used in cases where multiple work items are available at once:

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

## Senders and Receivers Represent Generic, Lazy Work

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

Without a task scheduling abstraction that offers such controls, it is not
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
to create async task chains in C++ and elsewhere. More modern codebases have
switched to using some variation of the promise/future abstraction that support
continuations (e.g., `std::experimental::future::then`). In C++20 and beyond,
we could even imagine standardizing on coroutines, so that
launching an async operation returns an awaitable. Each of these
approaches has strengths and weaknesses.

**Promises and futures**, as traditionally realized, require the dynamic
allocation and management of a shared control block, some form of
synchronization, and typically type-erasure of both the task and the
continuation. Many of these costs are inherent in the nature of “future” as a
handle to an operation that is already scheduled for execution. These expenses
rule out the promise/future abstraction for many uses and makes it a poor
choice for a basis of a Generic mechanism.

**Coroutines** have many of the same problems as promise/future except that, by
typically starting suspended, they avoid the need for synchronization when
chaining a dependent task. In many cases the coroutine frame is dynamically
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
creating task chains, but they are not without their own problems. Errors
happen, so there is a need to propagate either an error or a value. This simple
requirement has led to many different callback shapes. The lack of a standard
callback signature stands as an obstacle to Generic design. Additionally, few
if any of these shapes accommodate the need to also propagate a cancellation
signal when the user has requested an upstream task to stop and clean up.

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

Conceptually, a `sender` is a task that has not been scheduled for execution
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

Appendix B has the source of the `retry` algorithm. Note that the signature of
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

## Changelog

### Revision 12

Introduced introductory design discussion which replaces the obsolete [P0761](https://wg21.link/P0761).

### Revision 11

As directed by SG1 at the 2019-07 Cologne meeting, we have implemented the following changes suggested by P1658 and P1660 which incorporate "lazy" execution:

* Eliminated all interface-changing properties.
* Introduced `set_value`, `set_error`, `set_done`, `execute`, `submit`, and `bulk_execute` customization point objects.
* Introduced `executor`, `executor_of`, `receiver`, `receiver_of`, `sender`, `sender_to`, `typed_sender`, and `scheduler` concepts.
* Renamed polymorphic executor to `any_executor`.
* Introduced `invocable_archetype`.
* Eliminated `OneWayExecutor` and `BulkOneWayExecutor` requirements.
* Eliminated `is_executor`, `is_oneway_executor`, and `is_bulk_oneway_executor` type traits.
* Eliminated interface-changing properties from `any_executor`.

### Revision 10

As directed by LEWG at the 2018-11 San Diego meeting, we have migrated the property customization mechanism to namespace `std` and moved all of the details of its specification to a separate paper, [P1393](http://wg21.link/P1393).  This change also included the introduction of a separate customization point for interface-enforcing properties, `require_concept`.  The generalization also necessitated the introduction of `is_applicable_property_v` in the properties paper, which in turn led to the introduction of `is_executor_v` to express the applicability of properties in this paper.

### Revision 9

As directed by the SG1/LEWG straw poll taken during the 2018 Bellevue executors
meeting, we have separated The Unified Executors programming model proposal into two
papers. This paper contains material related to one-way execution which the
authors hope to standardize with C++20 as suggested by the Bellevue poll.
[P1244](http://wg21.link/P1244) contains remaining material related to
dependent execution. We expect P1244 to evolve as committee consensus builds
around a design for dependent execution.

This revision also contains bug fixes to the `allocator_t` property which were originally scheduled for Revision 7 but were inadvertently omitted.

### Revision 8

Revision 8 of this proposal makes interface-changing properties such as `oneway` mutually exclusive in order to simplify implementation requirements for executor adaptors such as polymorphic executors.
Additionally, this revision clarifies wording regarding execution agent lifetime.

### Revision 7

Revision 7 of this proposal corrects wording bugs discovered by the authors after Revision 6's publication.

* Enhanced `static_query_v` to result in a default property value for executors which do not provide a `query` function for the property of interest
* Revise `then_execute` and `bulk_then_execute`'s operational semantics to allow user functions to handle incoming exceptions thrown by preceding execution agents
* Introduce `exception_arg` to disambiguate the user function's exceptional overload from its nonexceptional overload in `then_execute` and `bulk_then_execute`

### Revision 6

Revision 6 of this proposal corrects bugs and omissions discovered by the authors after Revision 5's publication, and introduces an enhancement improving the safety of the design.

* Enforce mutual exclusion of behavioral properties via the type system instead of via convention
* Introduce missing `execution::require` adaptations
* Allow executors to opt-out of invoking factory functions when appropriate
* Various bug fixes and corrections

### Revision 5

Revision 5 of this proposal responds to feedback requested during the 2017 Albuquerque ISO C++ Standards Committee meeting and introduces changes which allow properties to better interoperate with polymorphic executor wrappers and also simplify `execution::require`'s behavior.

* Defined general property type requirements
* Elaborated specification of standard property types
* Simplified `execution::require`'s specification
* Enhanced polymorphic executor wrapper
    * Templatized `execution::executor<SupportableProperties...>`
    * Introduced `prefer_only` property adaptor
* Responded to Albuquerque feedback
    * From SG1
        * Execution contexts are now optional properties of executors
        * Eliminated ill-specified caller-agent forward progress properties
        * Elaborated `Future`'s requirements to incorporate forward progress
        * Reworded operational semantics of execution functions to use similar language as the blocking properties
        * Elaborated `static_thread_pool`'s specification to guarantee that threads in the bool boost-block their work
        * Elaborated operational semantics of execution functions to note that forward progress guarantees are specific to the concrete executor type
    * From LEWG
        * Eliminated named `BaseExecutor` concept
        * Simplified general executor requirements
        * Enhanced the `OneWayExecutor` introductory paragraph
        * Eliminated `has_*_member` type traits
* Minor changes
    * Renamed TS namespace from `concurrency_v2` to `executors_v1`
    * Introduced `static_query_v` enabling static queries
    * Eliminated unused `property_value` trait
    * Eliminated the names `allocator_wrapper_t` and `default_allocator`

### Revision 4

* Specified the guarantees implied by `bulk_sequenced_execution`, `bulk_parallel_execution`, and `bulk_unsequenced_execution`

### Revision 3

* Introduced `execution::query()` for executor property introspection
* Simplified the design of `execution::prefer()`
* `oneway`, `twoway`, `single`, and `bulk` are now `require()`-only properties
* Introduced properties allowing executors to opt into adaptations that add blocking semantics
* Introduced properties describing the forward progress relationship between caller and agents
* Various minor improvements to existing functionality based on prototyping

### Revision 2

* Separated wording from explanatory prose, now contained in paper [P0761](https://wg21.link/P0761)
* Applied the simplification proposed by paper [P0688](https://wg21.link/P0688)

### Revision 1

* Executor category simplification
* Specified executor customization points in detail
* Introduced new fine-grained executor type traits
    * Detectors for execution functions
    * Traits for introspecting cross-cutting concerns
        * Introspection of mapping of agents to threads
        * Introspection of execution function blocking behavior
* Allocator support for single agent execution functions
* Renamed `thread_pool` to `static_thread_pool`
* New introduction

### Revision 0

* Initial design

## Appendix A: Executors Bibilography

------
Paper                                                                                                     Notes                                                                                                                                                                           Date introduced
----------------                                                                                          ----------------                                                                                                                                                                ----------------
[N3378 - A preliminary proposal for work executors](https://wg21.link/N3378)\                             Initial executors proposal from Google, based on an abstract base class.                                                                                                        2012-02-24       
[N3562 - Executors and schedulers, revision 1](https://wg21.link/N3562)\                                                                                                                                                                                                                   
[N3731 - Executors and schedulers, revision 2](https://wg21.link/N3371)\                                                                                                                                                                                                                                   
[N3785 - Executors and schedulers, revision 3](https://wg21.link/N3785)\                                                                                                                                                                                                                                   
[N4143 - Executors and schedulers, revision 4](https://wg21.link/N4046)\                                                                                                                                                                                                                                   
[N4414 - Executors and schedulers, revision 5](https://wg21.link/N4414)\                                                                                                                                                                                                                                   
[P0008 - C++ Executors](https://wg21.link/P0008)                                                                                                                                                                                                                                          
                                                                                                                                                                                                                                                                                                           
[N4046 - Executors and Asynchronous Operations](https://wg21.link/N4046)                                  Initial executors proposal from Kohlhoff, based on extensions to ASIO.                                                                                                          2014-05-26       
                                                                                                                                                                                                                                                                                                           
[N4406 - Parallel Algorithms Need Executors](https://wg21.link/N4406)\                                    Initial executors proposal from Nvidia, based on a traits class.                                                                                                                2015-04-10       
[P0058 - An interface for abstracting execution](https://wg21.link/P0058)                                                                                                                                                                                                                                  
                                                                                                                                                                                                                                                                                                           
[P0285 - Using customization points to unify executors](https://wg21.link/P0285)                          Proposes unifying various competing executors proposals via customization points.                                                                                               2016-02-14       
                                                                                                                                                                                                                                                                                                           
[P0443 - A Unified Executors Proposal for C++](https://wg21.link/P0443)                                   This proposal.                                                                                                                                                                  2016-10-17       
                                                                                                                                                                                                                                                                                        
[P0688 - A Proposal to Simplify the Executors Design](https://wg21.link/P0688)                            Proposes simplifying this proposal's APIs using properties.                                                                                                                     2017-06-19       
                                                                                                                                                                                                                                                                                                 
[P0761 - Executors Design Document](https://wg21.link/P0761)                                              Describes the design of this proposal circa 2017.                                                                                                                               2017-07-31       
                                                                                                                                                                                                                                                                                                   
[P1055 - A Modest Executor Proposal](https://wg21.link/P1055)                                             Initial executors proposal from Facebook, based on lazy execution.                                                                                                              2018-04-26       
                                                                                                                                                                                                                                                                                        
[P1194 - The Compromise Executors Proposal: A lazy simplification of P0443](https://wg21.link/P1194)      Initial proposal to integrate senders and receivers into this proposal.                                                                                                         2018-10-08       
                                                                                                                                                                                                                                                                                        
[P1232 - Integrating executors with the standard library through customization](https://wg21.link/P1232)  Proposes to allow executors to customize standard algorithms directly.                                                                                                          2018-10-08       
                                                                                                                                                                                                                                                                                                           
[P1341 - Unifying asynchronous APIs in C++ standard Library](https://wg21.link/P1341)                     Identifies various incompatibilities between executors, senders, receivers, coroutines, and parallel algorithms.                                                                2018-11-25       
                                                                                                                                                                                                                                                                                                           
[P1393 - A General Property Customization Mechanism](https://wg21.link/P1393)                             Standalone paper proposing the property customization used by P0443 executors.                                                                                                  2019-01-13       
                                                                                                                                                                                                                                                                                                           
[P1677 - Cancellation is not an Error](https://wg21.link/P1677)                                           Motivates the need for `done` in addition to `error`.                                                                                                                           2019-05-18       
                                                                                                                                                                                                                                                                                                           
[P1678 - Callbacks and Composition](https://wg21.link/P1678)                                              Argues for callbacks/receivers as a universal design pattern in the standard library.                                                                                           2019-05-18       
                                                                                                                                                                                                                                                                                                           
[P1525 - One-Way execute is a Poor Basis Operation](https://wg21.link/P1525)                              Identifies deficiencies of `execute` as a basis operation.                                                                                                                      2019-06-17       
                                                                                                                                                                                                                                                                                        
[P1658 - Suggestions for Consensus on Executors](https://wg21.link/P1658)                                 Suggests progress-making changes to this proposal circa 2019.                                                                                                                   2019-06-17       
                                                                                                                                                                                                                                                                                        
[P1660 - A Compromise Executor Design Sketch](https://wg21.link/P1660)                                    Proposes concrete changes to this proposal along the lines of [P1525](https://wg21.link/P1525), [P1658](https://wg21.link/P1658), and [P1738](https://wg21.link/P1738).         2019-06-17       
                                                                                                                                                                                                                                                                                        
[P1738 - The Executor Concept Hierarchy Needs a Single Root](https://wg21.link/P1738)                     Identifies problems caused by a multi-root executor concept hierarchy.                                                                                                          2019-06-17       
                                                                                                                                                                                                                                                                                                           
[P1897 - Towards C++23 executors: A proposal for an initial set of algorithms](https://wg21.link/P1897)   Initial proposal for a set of customizable sender algorithms.                                                                                                                   2019-10-06       
                                                                                                                                                                                                                                                                                                           
[P1898 - Forward progress delegation for executors](https://wg21.link/P1898)                              Proposes a model of forward progress for executors and asynchronous task graphs.                                                                                                2019-10-06       
                                                                                                                                                                                                                                                                                                           
[P2006 - Splitting submit() into connect()/start()](https://wg21.link/P2006)                              Proposes refactoring `submit` into more fundamental `connect` and `start` sender operations.                                                                                    2020-01-13       
-----

## Appendix B: The `retry` Algorithm

Below is an implementation of a simple `retry` algorithm in terms of `sender`/`receiver`. This algorithm is Generic in the sense that it will retry any multi-shot asynchronous operation that satisfies the `sender` concept. More accurately, it takes any deferred async operation and wraps it so that when it is executed, it will retry the wrapped operation until it either succeeds or is cancelled.

Full working code can be found here: [https://godbolt.org/z/nm6GmH](https://godbolt.org/z/nm6GmH)

```P0443
// _conv needed so we can emplace construct non-movable types into
// a std::optional.
template<invocable F>
    requires std::is_nothrow_move_constructible_v<F>
struct _conv {
    F f_;
    explicit _conv(F f) noexcept : f_((F&&) f) {}
    operator invoke_result_t<F>() && {
        return ((F&&) f_)();
    }
};

// pass through set_value and set_error, but retry the operation
// from set_error.
template<class O, class R>
struct _retry_receiver {
    O* o_;
    explicit _retry_receiver(O* o): o_(o) {}
    template<class... As>
        requires receiver_of<R, As...>
    void set_value(As&&... as) &&
        noexcept(is_nothrow_receiver_of_v<R, As...>) {
        ::set_value(std::move(o_->r_), (As&&) as...);
    }
    void set_error(auto&&) && noexcept {
        o_->_retry(); // This causes the op to be retried
    }
    void set_done() && noexcept {
        ::set_done(std::move(o_->r_));
    }
};

template<sender S>
struct _retry_sender : sender_base {
    S s_;
    explicit _retry_sender(S s): s_((S&&) s) {}

    // Hold the nested operation state in an optional so we can
    // re-construct and re-start it when the operation fails.
    template<receiver R>
    struct _op {
        S s_;
        R r_;
        std::optional<state_t<S&, _retry_receiver<_op, R>>> o_;

        _op(S s, R r): s_((S&&)s), r_((R&&)r), o_{_connect()} {}
        _op(_op&&) = delete;

        auto _connect() noexcept {
            return _conv{[this] {
                return ::connect(s_, _retry_receiver<_op, R>{this});
            }};
        }
        void _retry() noexcept try {
            o_.emplace(_connect()); // potentially throwing
            ::start(std::move(*o_));
        } catch(...) {
            ::set_error((R&&) r_, std::current_exception());
        }
        void start() && noexcept {
            ::start(std::move(*o_));
        }
    };

    template<receiver R>
        requires sender_to<S&, _retry_receiver<_op<R>, R>>
    auto connect(R r) && -> _op<R> {
        return _op<R>{(S&&) s_, (R&&) r};
    }
};

template<sender S>
sender auto retry(S s) {
    return _retry_sender{(S&&)s};
}
```


# Proposed Wording

