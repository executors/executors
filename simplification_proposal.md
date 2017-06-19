At the Kona meeting of the ISO C++ Standards Committee, we presented a design
for executors, which we envision to be components for creating execution in
C++. Our design, described in [P0443R1](https://wg21.link/P0443R1), was a
unification of three independent proposals targeted at different use cases.
Those use cases are the execution-creating interfaces of the Standard Library
(e.g., `async`), as well as interfaces found in the Concurrency, Parallelism,
and Networking Technical Specifications. We believe P0443R1's functionality
is necessary to support them.

Discussion in Kona made it clear that the design of P0443R1 was too complex. We
agree, and have considered possible simplifications. One approach toward
reducing complexity would discard functionality. However, we do not believe it
is possible to eliminate functionality without compromising interoperability
with the use cases we have committed to supporting. Moreover, simply
eliminating features would only reduce complexity today. Those features and the
complexity they bring could return tomorrow.

Instead, we believe a successful approach will be to refactor P0443R1's
functionality into a more manageable, factored form based on *executor
properties*, which are user-requestable behaviors which modify the way
executors create execution. Compared to P0443R1's design, we believe a
property-based design will be simpler both for executor clients to use and for
executor authors to implement. Because our properties design is open-ended, it
may be extended with new properties. Finally, this design's specification is
much more compact than P0443R1. As a consequence, we have been able to
quickly produce an [open source prototype](https://github.com/executors/issaquah_2016/tree/rebind-prototype/rebind_prototype)
and [several example programs](https://github.com/executors/issaquah_2016/tree/rebind-prototype/rebind_prototype/examples).

**Introductory examples.** To briefly summarize by example, higher-level, indirect use of executors through control structures remains the same as with P0443R1:

    // execute an async on an executor:
    auto future = std::async(my_executor, task1);

    // execute a parallel for_each on an executor:
    std::for_each(std::execution::par.on(my_executor), data.begin(), data.end(), task2);

Lower-level, direct use of executors through execution functions changes to use requirements and preferences:
 
    // make require(), prefer(), and properties available
    using namespace std::experimental::execution;

    // execute a non-blocking, fire-and-forget task on an executor:
    require(my_executor, oneway, never_blocking).execute(task1);

    // execute a non-blocking, two-way task on an executor. prefer to execute as a continuation:
    auto future2 = prefer(require(my_executor, twoway), continuation).twoway_execute(task2);

    // when future is ready, execute a possibly-blocking 10-agent task
    auto bulk_exec = require(my_executor, possibly_blocking, bulk, then);
    auto future3 = bulk_exec.bulk_then_execute(task3, 10, future2, result_factory, shared_factory);

# Proposed Simplification

The complexity of P0443R1's design emanates from the execution functions
comprising the fundamental API for creating work with executors. This set of
functions is the result of the cross product of three sets of properties:
blocking behavior (i.e., never-blocking, possibly-blocking, and
    always-blocking), cardinality (i.e. single and bulk), and directionality
(one-way and two-way). A partial cross product yields the set of sixteen
execution functions identified by P0443R1. Rather than "hard-code" a set of
arbitrary combinations into the possible executor behaviors, our proposed
simplification allows the user to programmatically build an executor with the
desired behavior. This refactoring allows us to reduce the set of execution functions.

## Execution Functions

TODO: There are six of these: `execute`, `twoway_execute`, `then_execute`, `bulk_execute`, `bulk_twoway_execute`, and `bulk_then_execute`.

explain that these retain their original meanings from P0443R1

## User Requirements

For example, suppose a user requires to create an execution agent with two-way,
    non-blocking execution. Under the API specified by P0443R1, this is
    accomplished by using the specific customization point tasked with
    implementing this combination of properties:

    using namespace std::experimental::execution;
    auto future = async_post(exec, task);

In our proposed design, the user separately *requires* the properties of interest, and then calls an execution function:

    using namespace std::experimental::execution;
    auto future = require(exec, twoway, never_blocking).twoway_execute(task);

The `require()` call returns a new executor adapting the given executor's
native behavior to guarantee the required behaviors. If the given executor's
native behavior already provides the required guarantee, then `require()`
behaves like the identity function and returns the executor unchanged. If it is
not possible to satisfy a requirement, then it is a compile-time error.

As another example, suppose a user requires to create an execution agent with
one-way, blocking execution. This is not possible with P0443R1's API,
because it does not specify an execution function for this combination of
requirements. However, our new proposed design does permit this combination:

    using namespace std::experimental::execution;
    require(exec, oneway, always_blocking).execute(task);

## User Preferences

Some of the properties users desire of executors are not hard requirements.
Instead, they are softer *preferences*. For example, suppose a user requires to
create a one-way, never-blocking execution agent and for performance reasons,
would prefer that the agent execute as a continuation of the calling
thread. P0443R1 defines a special execution function named `defer()` for
this purpose:

    using namespace std::experimental::execution;
    defer(exec, task);

As described by P0443R1, `defer()`'s semantics are equivalent to `post()`'s.
The distinction is that `defer()` acts as a hint to the executor to create the
execution agent in a particular way. The presence of `defer()` in P0443R1 was controversial.

Our new proposed design introduces `prefer()` as an avenue for communicating such hints:

    using namespace std::experimental::execution;
    require(exec, oneway).prefer(is_continuation).execute(task);

Unlike requirements, executors are under no obligation to satisfy user
preferences. If it is not possible to satisfy a preference, then it is not a
compile-time error.

## Executor Properties

first: list desirable properties of properties, can take some text from ChrisK's issue about a hinting mechanism

### Proposed Properties

Our proposal includes eight sets of properties we have identified as necessary
to supporting the needs of the Standard Library and other technical
specifications. Two of these sets describe the directionality and cardinality
of execution member functions which create execution. When a user requests
these properties, they are implicitly requesting an executor which provides the
execution functions implied by the request.

**Directionality.** The directionality properties we propose are `oneway`, `twoway`, and `then`. An
executor with the `oneway` property has either or both of the one-way execution
functions: `.execute()` or `.bulk_execute()`. An executor with the `twoway`
property has either or both of the two-way execution functions:
`.twoway_execute()` or `.bulk_twoway_execute()`. An executor with the `then`
property has either or both of the `then_` execution functions:
`then_execute()` or `bulk_then_execute()`. Because a single executor type can
have one or more of these member functions all at once, these properties are
not mutually exclusive.

**Cardinality.** There are two cardinality properties: `single` and `bulk`. An executor with the
`single` property has at least one execution function which creates a single
execution agent from a single call. Likewise, an executor with the `bulk`
property has at least one execution function which creates multiple execution
agents in bulk from a single call. Like the directionality properties, the
cardinality properties are not mutually exclusive, because it is possible for
a single executor type to have both kinds of execution functions.

**Blocking.** There are three mutually-exclusive blocking properties :
`never_blocking`, `possibly_blocking`, and `always_blocking`. Unlike the
directionality and cardinality properties, which imply the existence of certain
execution functions, the blocking properties instead guarantee the blocking
behavior of those member functions. For example, when `.execute(task)`
is called on an executor whose blocking property is `never_blocking`, then the
forward progress of the calling thread will never be blocked pending the
completion of the execution agent created by the call. The same guarantee holds
for every other execution function of that executor. The net effect is that,
unlike in P0443R1, the blocking behavior of execution functions is
completely a property of the executor type. However, that property can be
changed at will by transforming the executor into a different type through
a call to `require()`.

**Continuations.** There are two mutually-exclusive properties for indicate that a task submitted
to an executor represents a continuation of the calling thread: `continuation`
and `not_continuation`. A client may use the `continuation` property to
indicate that a program may execute more efficiently when tasks are
executed as continuations.

**Future task submission.** There are two mutually-exclusive properties to
indicate the likelihood of additional task submission in the future. The
`outstanding_work` work property indicates to an executor that additional task
submission is likely. Likewise, the `not_outstanding_work` property indicates
that no outstanding work remains.

**Bulk forward progress guarantees.** There are three mutually exclusive
properties which describe the forward progress guarantees of execution agents
created in bulk. These describe the forward progress of an agent with respect
to the other agents created in the same submission. These are
`bulk_sequenced_execution`, `bulk_parallel_execution`, and
`bulk_unsequenced_execution`, and they correspond to the three standard
execution policies.

**Thread execution mapping guarantees.** There are two mutually exclusive
properties for describing the way in which execution agents are mapped onto
threads. `thread_execution_mapping` guarantees that execution agents are mapped
onto threads of execution, while `new_thread_execution_mapping` extends that
guarantee by guaranteeing that each execution agent will be executed on a
newly-created individual thread. These guarantees may be used by the client to
reason about the existence and sharing of thread-local storage over an
execution agent's lifetime.

**Allocators.** A final property, `allocator`, associates an allocator with an
executor. A client may use this property to require the use of a preferred
allocator when allocating storage necessary to create execution. Of the
properties we propose to introduce, `allocator(alloc)` is the only one which takes an
additional parameter; namely, the desired allocator to use.

# Usage Examples

## Using executors with control structures

nothing really changes compared to P0443R1

## Implementing control structures

Demonstrate the use of `require()` & `prefer()`

Show what it looks like before & after the proposed simplification

# Acknowledgements

# Approximate Proposed Wording

What ChrisK is working on goes here

