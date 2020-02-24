% A Unified Executors Proposal for C++ | P0443R13

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

Document Number:    P0443R13

Date:               2020-03-02

Audience:           SG1 - Concurrency and Parallelism, LEWG

Reply-to:           sg1-exec@googlegroups.com

Abstract:           This paper proposes [a programming model](#proposed-wording) for executors, which are modular components for creating execution, and senders, which are lazy descriptions of execution.

------------------------------------------------------
