\textcolor{red}{This outline is just off the top of my head, edit freely}

At the Kona meeting of the ISO C++ Standards Committee, we presented a design
for executors, which we envision to be components for creating execution in
C++. Our design, described in [P0443R1](https://wg21.link/P0443R1), was a
unification of three independent proposals targeted at different use cases.
Those use cases are the execution-creating interfaces of the Standard Library
(e.g., `async`), as well as interfaces found in the Concurrency, Parallelism,
and Networking Technical Specificationss. We believe the functionality
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

Short summary of simplification:

Note that the simplification is intended to preserve the functionality of P0443R1

## Fundamental work submission functions

There should be four

## Executor Properties

list some important properties
perhaps note that the old `defer` behavior is now a property

note how we distinguish properties from the fundamental work submission functions -- the submission functions have parameters with semantic meaning to the work being created

## Requesting Properties

`require()`vs `prefer()`

# Usage Examples

## Using executors with control structures

nothing really changes compared to P0443R1

## Implementing control structures

Demonstrate the use of `require()` & `prefer()`

Show what it looks like before & after the proposed simplification

# Approximate Proposed Wording

What ChrisK is working on goes here

