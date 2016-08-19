# issaquah_2016
A proposal for a minimal executor model for the Issaquah 2016 ISO C++ committee meeting

# Front Matter

TODO

# Minimal executor category

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

### Classifying forward progress guarantees of executor operations

    // XXX this section could use a bikeshed

    struct possibly_blocking_execution_tag {};
    struct blocking_execution_tag {};
    struct nonblocking_execution_tag {};
    
    template<class Executor>
    struct executor_operation_forward_progress
    {
      private:
        // exposition only
        template<class T>
        using __helper = typename T::operation_forward_progress;
    
      public:
        using type = std::experimental::detected_or_t<__helper, possibly_blocking_execution_tag>;
    };

    template<class Executor>
    using executor_operation_forward_progress_t = typename executor_operation_forward_progress<Executor>::type;

## `Executor`

1. The `Executor` requirements form the basis of the executor concept taxonomy;
   every executor satisfies the `Executor` requirements. This set of
   requirements specifies operations for creating execution agents.

2. In Table \ref{executor_requirements}, `f` denotes a `MoveConstructible` function object with zero arguments whose result type is `R`,
   and `x` denotes an object of type `X`.

3. A type `X` satisfies the `Executor` requirements if:
  * `X` satisfies the `CopyConstructible` requirements (17.6.3.1).
  * For any `f` and `x`, at least one of the expressions in Table \ref{executor_requirements} are valid and have the indicated semantics.

Table: (Executor requirements) \label{executor_requirements}


| Expression                       | Return Type              |  Operational semantics                          | Assertion/note/pre-/post-condition                                                                                              |
|----------------------------------|--------------------------|-------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| `x.spawn_execute(std::move(f))`  | `void`                   |  Creates an execution agent which invokes `f()` | Effects: blocks the forward progress of the caller until `f` is finished as given by `executor_operation_forward_progress_t<X>` |
| `x.async_execute(std::move(f))`  | `executor_future_t<X,R>` |  Creates an execution agent which invokes `f()` | Effects: blocks the forward progress of the caller until `f` is finished as given by `executor_operation_forward_progress_t<X>` |

### Classifying forward progress guarantees of groups of execution agents

    struct sequenced_execution_tag {};
    struct parallel_execution_tag {};
    struct unsequenced_execution_tag {};

    // TODO: we'll want this type in the future, but it is not
    /        required for any functionality of version 0
    // struct concurrent_execution_tag {};

# Bulk (i.e., Parallelism TS) executor category

## Bulk executor traits

### Associated shape type

    template<class Executor>
    struct executor_shape
    {
      private:
        // exposition only
        template<class T>
        using __helper = typename T::shape_type;
    
      public:
        using type = std::experimental::detected_or_t<__helper, size_t>;

        // exposition only
        static_assert(std::is_integral_v<type>, "shape type must be an integral type");
    };

    template<class Executor>
    struct executor_index
    {
      private:
        // exposition only
        template<class T>
        using __helper = typename T::index_type;

      public:
        using type = std::experimental::detected_or_t<__helper, executor_shape_t<Executor>>;

        // exposition only
        static_assert(std::is_integral_v<type>, "index type must be an integral type");
    };

## BulkExecutor

1. The `BulkExecutor` requirements form the basis of the bulk executor concept taxonomy.
   This set of requirements specifies operations for creating groups of execution agents in bulk from a single operation.

2. In Table \ref{bulk_executor_requirements},
    * `f` denotes a `CopyConstructible` function object with three arguments,
    * `n` denotes a shape object whose type is `executor_shape_t<X>`.
    * `rf` denotes a `CopyConstructible` function object with one argument whose result type is `R`,
    * `sf` denotes a `CopyConstructible` function object with one argument whose result type is `S`,
    * `i` denotes an object whose type is `executor_index_t<X>`,
    * `r` denotes an object whose type is `R`, and
    * `s` denotes an object whose type is `S`.

2. A class `X` satisfies the requirements of a bulk executor if `X` satisfies
   the `Executor` requirements and the expressions of Table
   \ref{bulk_executor_requirements} are valid and have the indicated semantics.

Table: (Bulk executor requirements) \label{bulk_executor_requirements}

| Expression                               | Return Type              |  Operational semantics                                                      | Assertion/note/pre-/post-condition                                                                                                                     |
|------------------------------------------|--------------------------|-----------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
| `x.bulk_execute(f, n, rf, sf)`           | `R`                      |  Creates a group of execution agents of shape `n` which invoke `f(i, r, s)` | Note: blocks the forward progress of the caller until all invocations of `f` are finished.                                                             |
| `x.bulk_async_execute(f, n, rf, sf)`     | `executor_future_t<X,R>` |  Creates a group of execution agents of shape `n` which invoke `f(i, r, s)` | Effects: blocks the forward progress of the caller until all invocations of `f` are finished as required by `executor_operation_forward_progress_t<X>` |
| `x.bulk_then_execute(f, n, pred, rf, sf)`| `executor_future_t<X,R>` |  Creates a group of execution agents of shape `n` which invoke `f(i, r, s)` | Effects: blocks the forward progress of the caller until all invocations of `f` are finished as required by `executor_operation_forward_progress_t<X>` |

