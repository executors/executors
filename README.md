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

### Executor type requirements

Executors are `CopyConstructible` types that create execution agents through executor operations.

Calling an executor operation affects the forward progress of the calling thread as given by `executor_operation_forward_progress_t<Executor>`.

The rows of table \ref{executor_operation_requirements} specify executor operations. An executor class shall provide at least one of these operations.

In table \ref{executor_operation_requirements}, `X` denotes an executor class object, `x` denotes a value of type `X`, `f` denotes a `MoveConstructible`
function object with zero arguments, and `R` denotes the type of `f()`.

For any `f` and `x`, at least one of the operations of table \ref{executor_operation_requirements} shall be well-formed.

Table: (Executor operation requirements) \label{executor_operation_requirements}

------------------------------------------------------------------------------------------------
 Expression                        Return Type                Operational semantics             
--------------------------------- -------------------------- -----------------------------------
 `x.spawn_execute(std::move(f))`   `void`                     Creates a one-way execution agent 
 `x.async_execute(std::move(f))`   `executor_future_t<X,R>`   Creates a two-way execution agent 
------------------------------------------------------------------------------------------------

