## Changelog

### Revision 12

Introduced introductory design discussion which replaces the obsolete [P0761](https://wg21.link/P0761). No normative changes.

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

## Appendix: Executors Bibilography

------
Paper                                                                                                     Notes                                                                                                                                                                           Date introduced
----------------                                                                                          ----------------                                                                                                                                                                ----------------
[N3378 - A preliminary proposal for work executors](https://wg21.link/N3378)\                             Initial executors proposal from Google, based on an abstract base class.                                                                                                        2012-02-24       
[N3562 - Executors and schedulers, revision 1](https://wg21.link/N3562)\                                                                                                                                                                                                                   
[N3731 - Executors and schedulers, revision 2](https://wg21.link/N3371)\                                                                                                                                                                                                                                   
[N3785 - Executors and schedulers, revision 3](https://wg21.link/N3785)\                                                                                                                                                                                                                                   
[N4143 - Executors and schedulers, revision 4](https://wg21.link/N4143)\                                                                                                                                                                                                                                   
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

[P1244 - Dependent Execution for a Unified Executors Proposal for C++](https://wg21.link/P1244)           Vestigal futures-based dependent execution functionality excised from later revisions of this proposal.                                                                         2018-10-08
                                                                                                                                                                                                                                                                                                           
[P1341 - Unifying asynchronous APIs in C++ standard Library](https://wg21.link/P1341)                     Proposes enhancements making senders awaitable.                                                                                                                                 2018-11-25       
                                                                                                                                                                                                                                                                                                           
[P1393 - A General Property Customization Mechanism](https://wg21.link/P1393)                             Standalone paper proposing the property customization used by P0443 executors.                                                                                                  2019-01-13       
                                                                                                                                                                                                                                                                                                           
[P1677 - Cancellation is serendipitous-success](https://wg21.link/P1677)                                  Motivates the need for `done` in addition to `error`.                                                                                                                           2019-05-18       
                                                                                                                                                                                                                                                                                                           
[P1678 - Callbacks and Composition](https://wg21.link/P1678)                                              Argues for callbacks/receivers as a universal design pattern in the standard library.                                                                                           2019-05-18       
                                                                                                                                                                                                                                                                                                           
[P1525 - One-Way execute is a Poor Basis Operation](https://wg21.link/P1525)                              Identifies deficiencies of `execute` as a basis operation.                                                                                                                      2019-06-17       
                                                                                                                                                                                                                                                                                        
[P1658 - Suggestions for Consensus on Executors](https://wg21.link/P1658)                                 Suggests progress-making changes to this proposal circa 2019.                                                                                                                   2019-06-17       
                                                                                                                                                                                                                                                                                        
[P1660 - A Compromise Executor Design Sketch](https://wg21.link/P1660)                                    Proposes concrete changes to this proposal along the lines of [P1525](https://wg21.link/P1525), [P1658](https://wg21.link/P1658), and [P1738](https://wg21.link/P1738).         2019-06-17       
                                                                                                                                                                                                                                                                                        
[P1738 - The Executor Concept Hierarchy Needs a Single Root](https://wg21.link/P1738)                     Identifies problems caused by a multi-root executor concept hierarchy.                                                                                                          2019-06-17       
                                                                                                                                                                                                                                                                                                           
[P1897 - Towards C++23 executors: A proposal for an initial set of algorithms](https://wg21.link/P1897)   Initial proposal for a set of customizable sender algorithms.                                                                                                                   2019-10-06       
                                                                                                                                                                                                                                                                                                           
[P1898 - Forward progress delegation for executors](https://wg21.link/P1898)                              Proposes a model of forward progress for executors and asynchronous graphs of work.                                                                                             2019-10-06       
                                                                                                                                                                                                                                                                                                           
[P2006 - Splitting submit() into connect()/start()](https://wg21.link/P2006)                              Proposes refactoring `submit` into more fundamental `connect` and `start` sender operations.                                                                                    2020-01-13       

[P2033 - History of Executor Properties](https://wg21.link/P2033)                                         Documents the evolution of [P1393](https://wg21.link/P1393)'s property system, especially as it relates to executors.                                                           2020-01-13
-----

## Appendix: A note on coroutines

[P1341](http://wg21.link/P1341) leverages the structural similarities between
coroutines and the sender/receiver abstraction to give a class of senders a
standard-provided `operator co_await`. The end result is that a sender, simply
by dint of being a sender, can be `co_await`-ed in a coroutine. With the
refinement of sender/receiver being proposed in
[P2006](https://wg21.link/P2006) — namely, the splitting of `submit` into
`connect`/`start` — that automatic adaptation from sender-to-awaitable is
allocation- and synchronization-free.


## Appendix: The `retry` Algorithm

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

