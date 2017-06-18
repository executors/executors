\textcolor{red}{This outline is just off the top of my head, edit freely}

At the Kona meeting of the ISO C++ Standards Committee, we presented a design
for executors, which we envision to be components for creating execution in
C++. Our design, described in [P0443R1](https://wg21.link/P0443R1), was a
unification of three independent proposals targeted at different use cases.
Those use cases are the execution-creating interfaces of the Standard Library
(e.g., `async`), as well as interfaces found in the Concurrency, Parallelism,
and Networking Technical Specifications. We believe the functionality
offered by P0443R1 is necessary to fulfill the requirements of those
interfaces for interoperating with executors.

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
property-based design will be easier both for executor clients to use and for
executor authors to implement. Because our properties design is open-ended, it
may be extended in a straightforward, scalable way by inventing new properties
in the future. Finally, we believe that the specification required by this
design will be much more compact compared to P0443R1. As a consequence, we have
been able to quickly produce an [open source prototype](https://github.com/executors/issaquah_2016/tree/rebind-prototype/rebind_prototype)
and several [example programs](https://github.com/executors/issaquah_2016/tree/rebind-prototype/rebind_prototype/examples).

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
desired behavior.

## Execution Functions

There should be six of these: `oneway_execute`, `twoway_execute`, `then_execute`, `bulk_oneway_execute`, `bulk_twoway_execute`, and `bulk_then_execute`.

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
behaves like the identify function and returns the executor unchanged. If it is
not possible to satisfy a requirement, then it is a compile-time error.

As another example, suppose a user requires to create an execution agent with
one-way, blocking execution. This is not possible with P0443R1's API,
because it does not specify an execution function for this combination of
requirements. However, our new proposed design does permit this combination:

    using namespace std::experimental::execution;
    require(exec, oneway, always_blocking).oneway_execute(task);

## User Requirements

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
    require(exec, oneway).prefer(is_continuation).oneway_execute(task);

Unlike requirements, executors are under no obligation to satisfy user
preferences. If it is not possible to satisfy a preference, then it is not a
compile-time error.

## Executor Properties

Our proposal includes eight sets of properties we have identified as necessary
to supporting the needs of the Standard Library and other technical
specifications. Two of these sets control the directionality and cardinality
of execution member functions which create execution.

list the ones we'd like to specify now to support the stdlib and TSes

list desirable properties of properties, can take some text from ChrisK's issue about a hinting mechanism

note how we distinguish properties from the fundamental work submission functions -- the submission functions have parameters with semantic meaning to the work being created

# Usage Examples

## Using executors with control structures

nothing really changes compared to P0443R1

## Implementing control structures

Demonstrate the use of `require()` & `prefer()`

Show what it looks like before & after the proposed simplification

# Acknowledgements

# Approximate Proposed Wording

What ChrisK is working on goes here

