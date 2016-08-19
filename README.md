# issaquah_2016
A proposal for a minimal executor model for the Issaquah 2016 ISO C++ committee meeting

# Front Matter

TODO

# Executor

## Executor traits

### Associated future type

    template<class Executor, class T>
    struct executor_future
    {
      // TODO: elaborate this later to allow executor-specific future types
      using type = std::future<T>;
    };
    
    template<class Executor, class T>
    using executor_future_t = typename executor_future<Executor,T>::type;

### Forward progress guarantees of executor operations

    struct operation_does_not_block {};
    
    struct operation_does_block {};
    
    struct operation_may_block {};
    
    // XXX this trait needs a bikeshed
    template<class Executor>
    struct executor_operation_forward_progress
    {
      private:
        // exposition only
        template<class T>
        using __helper = typename T::operation_forward_progress;
    
      public:
        using type = std::experimental::detected_or_t<__helper, operation_may_block>;
    };

    template<class Executor>
    using executor_operation_forward_progress_t = typename executor_operation_forward_progress<Executor>::type;

### Executor

1. The `Executor` requirements form the basis of the executor concept taxonomy;
   every executor satisfies the `Executor` requirements. This set of
   requirements specifies operations for creating execution agents.

2. A type `X` satisfies the `Executor` requirements if:
  * `X` satisfies the `CopyConstructible` requirements (17.6.3.1).
  * For any `MoveConstructible` function object with zero arguments `f` and object `x` of type `X`,
    at least one of the expressions in Table \ref{executor_operations} are valid and have the indicated semantics.

Table: (Executor requirements) \label{executor_operation_requirements}


| Expression                       | Return Type              |  Operational semantics      | Assertion/note/pre-/post-condition                                                                                              |
|----------------------------------|--------------------------|-----------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| `x.spawn_execute(std::move(f))`  | `void`                   |  Creates an execution agent | Effects: blocks the forward progress of the caller until `f` is finished as given by `executor_operation_forward_progress_t<X>` |
| `x.async_execute(std::move(f))`  | `executor_future_t<X,R>` |  Creates an execution agent | Effects: blocks the forward progress of the caller until `f` is finished as given by `executor_operation_forward_progress_t<X>` |

