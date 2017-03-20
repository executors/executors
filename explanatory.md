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
* Define terms like “executor”, “execution resource”, and so on
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
