% Dependent Execution for a Unified Executors Proposal for C++ | P1244R0

----------------    -------------------------------------
Title:              Dependent Execution for a Unified Executors Proposal for C++

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

Document Number:    P1244R0

Date:               2018-10-08

Audience:           SG1 - Concurrency and Parallelism, LEWG

Reply-to:           sg1-exec@googlegroups.com

Abstract:           This paper extends [P0443](http://wg21.link/P0443)'s executors programming model with functionality to support dependent execution.

------------------------------------------------------

## Changelog

### Revision 0

Initial revision.

As directed by the SG1/LEWG straw poll taken during the 2018 Bellevue executors
meeting, we have separated The Unified Executors programming model proposal
into two papers. [P0443](http://wg21.link/P0443) contains material related to
one-way execution which the authors hope to standardize with C++20 as suggested
by the Bellevue poll. This document contains remaining material related to
dependent execution. We expect the form of this paper's material to evolve as
committee consensus builds around a design for dependent execution.

# Proposed Wording

