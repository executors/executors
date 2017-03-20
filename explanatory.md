Executors Explanatory Paper Outline

Paper should be no longer than 10 pages in a reasonable (10pt) font.

# Introduction
* Motivate the need for executors
* Cite prior papers and the companion wording document
* Should be written so that this companion paper can continue to be updated to
  explain the Executors TS as it evolves.  The idea would be to keep the
  explanation up-to-date so that the discussion of applying it to the IS can be
  informed by this paper as well.

# Terminology

Our proposed programming model introduces executors as a uniform interface for
creating execution that may not be common to the underlying execution resources
actually responsible for the mechanics of implementing that execution. There
are three major concepts involved in this interplay: execution resources,
execution contexts, and executors.

An **execution resource** is an instance of a hardware and/or software facility
capable of executing a callable function object. Different resources may offer
a broad array of functionality and semantics, and may range from SIMD vector
units accessible in a single thread to an entire runtime managing a large
collection of threads. In practice, different resources may also exhibit
different performance characteristics of interest to the performance-conscious
programmer. For example, an implementation might expose different processor
cores, with potentially non-uniform access to memory, as separate resources to
enable programmers to reason about locality.

An **execution context** is a program object that represents a specific
collection of execution resources.

An **executor** is an object associated with a specific execution context. It
provides a mechanism for creating execution agents from a callable function
object. The agents created are bound to the executor's context, and hence to
one or more of the resources that context represents.

Executors themselves are the primary concern of our design.

* If needed, expand into a more general Background section

# Using Executors
* Describe the application-level usage model we aim to support
* Demonstrate interaction with control structures like `std::async` and `std::for_each`
* Demonstrate use cases central to the Networking TS 
* A (strawman or proposed?) mechanism for using an executor that preserves the
  “legacy” meaning of `std::async`
* Demonstrate application-level library interface parameterized by executors.  Focus on
  something like a numerical solver, not on standard algorithms like `for_each`.
  Highlight compositional properties.
# Obtaining executors from a context like `{static,dynamic}_thread_pool`

# Building Control Structures
* Define what we mean by “control structure”.  Highlight examples from the Standard Library,
  Concurrency TS, Parallelism TS, and the Networking TS
* Control structures are clients of the executor framework
* Review the material necessary for a control structure author (e.g., someone who wants to
  implement a low-level parallel/concurrent mechanism)
* Survey the semantics of the various customization points.  This is the centerpiece of this
  section.

# Implementing Executors
* Survey low-level details that are not covered already and which the author of an executor
  type would need to know
* Detail the various groupings of execution functions:
  * The essentials:  `bulk_sync_execute`, `bulk_async_execute`, `bulk_async_post`,
    `bulk_then_execute`
  * Single-agent execution functions: `sync_execute`, `async_execute`, `async_post`,
    `then_execute`
  * One-way execution functions:  `bulk_execute`, `bulk_post`, `execute`, `post`
  * Support `defer` as well as `post`
* For each of the groupings after the first “essential” grouping, the corresponding
  subsection should be structured like this:
  * Show how the semantics of the corresponding customization point can be implemented by
    calling one of the previously defined methods
  * Provide an argument, preferably with empirical evidence, why doing so is insufficient and
    demonstrating how providing the more specialized method is a superior choice.
* Describe the adaptations performed by executor customization points
  * Highlight the costs implied by specific adaptations, e.g. temporary intermediate future
    creation or dynamic memory allocation
  * Discuss how the adaptations performed by customization points are chosen and in what order
    they are preferred
  * Discuss how blocking behavior interacts with customization points

# Future Work
* Quick survey of papers being written, or that probably ought to be written
* Should include:  dynamic thread pool, Future concept

# References

1.  [N3378 - A preliminary proposal for work executors](https://wg21.link/N3378), M. Austern et al., 2012-02-24.
2.  [N3562 - Executors and schedulers, revision 1](https://wg21.link/N3562), M. Austern et al., 2013-03-15.
3.  [N3731 - Executors and schedulers, revision 2](https://wg21.link/N3731), Chris Mysen and Niklas Gustafsson, 2013-08-25.
4.  [N3785 - Executors and schedulers, revision 3](https://wg21.link/N3785), C. Mysen et al., 2013-10-08.
5.  [N4046 - Executors and Asynchronous Operations](https://wg21.link/N4046), Christopher Kohlhoff, 2014-05-26.
6.  [N4406 - Parallel Algorithms Need Executors](https://wg21.link/N4406), J. Hoberock et al., 2015-04-10.
7.  [N4414 - Executors and schedulers, revision 5](https://wg21.link/N4414), Chris Mysen, 2015-04-10.
8.  [P0008R0 - C++ Executors](https://wg21.link/P0008R0), Chris Mysen, 2015-09-27.
9.  [P0058R0 - An Interface for Abstracting Execution](https://wg21.link/P0058R0), J. Hoberock et al., 2015-09-25.
10. [P0058R1 - An Interface for Abstracting Execution](https://wg21.link/P0058R1), J. Hoberock et al., 2016-02-12.
11. [P0285R0 - Using customization points to unify executors](https://wg21.link/P0285R0), Christopher Kohlhoff, 2016-02-14.
12. [P0443R0 - A Unified Executors Proposal for C++](https://wg21.link/P0443R0), J. Hoberock et al., 2016-10-17.
13. [P0443R1 - A Unified Executors Proposal for C++](https://wg21.link/P0443R1), J. Hoberock et al., 2017-01-06.

