% A Unified Executors Proposal for C++ | P0443R3

----------------    -------------------------------------
Title:              A Unified Executors Proposal for C++

Authors:            Jared Hoberock, jhoberock@nvidia.com

                    Michael Garland, mgarland@nvidia.com

                    Chris Kohlhoff, chris@kohlhoff.com

                    Chris Mysen, mysen@google.com

                    Carter Edwards, hcedwar@sandia.gov

                    Gordon Brown, gordon@codeplay.com

Other Contributors: Hans Boehm, hboehm@google.com

                    Thomas Heller, thom.heller@gmail.com

                    Lee Howes, lwh@fb.com

                    Bryce Lelbach, brycelelbach@gmail.com

                    Hartmut Kaiser, hartmut.kaiser@gmail.com

                    Bryce Lelbach, brycelelbach@gmail.com

                    Gor Nishanov, gorn@microsoft.com

                    Thomas Rodgers, rodgert@twrodgers.com

                    David Hollman, dshollm@sandia.gov

                    Michael Wong, michael@codeplay.com

Document Number:    P0443R3

Date:               2017-10-16

Audience:           SG1 - Concurrency and Parallelism

Reply-to:           sg1-exec@googlegroups.com

Abstract:           This paper proposes a programming model for executors, which are modular components for creating execution. The design of this proposal is described in paper [P0761](https://wg21.link/P0761).

------------------------------------------------------

## Changelog

### Revision 3

* Introduced `execution::query()` for executor property introspection
* Simplified the design of `execution::prefer()`
* `oneway`, `twoway`, `single`, and `bulk` are now `require()`-only properties
* Introduced properties allowing executors to opt into one-way to two-way adaptations
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

# Proposed Wording

