# Front Matter

## Conceptual Elements

**Instruction Stream:**
  Code to be run in a form appropriate for the target execution architecture.

**Execution Architecture:**
  Denotes the target architecture for an instruction stream.
  The instruction stream defined by the *main* entry point
  and associated global object initialization instruction streams
  is the *host process* execution architecture.
  Other possible target execution architectures include attached
  accelerators such as GPU, remote procedure call (RPC), and
  database management system (DBMS) servers.
  The execution architecture may impose architecture-specific constraints
  and provides architecture-specific facilities for an instruction stream.

**Execution Resource:**
  An instance of an execution architecture that is capable of running
  an instruction stream targeting that architecture.
  Examples include a collection of ``std::thread`` within the host process
  that are bound to particular cores, GPU CUDA stream, an RPC server,
  a DBMS server.
  Execution resources typically have weak *locality* properties both with
  respect to one another and with respect to memory resources.
  For example, cores within a non-uniform memory access (NUMA) region
  are *more local* to each other than cores in different NUMA regions
  and hyperthreads within a single core are more local to each other than
  hyperthreads in different cores.

*Lightweight* **Execution Agent:**
  An instruction stream is run by an execution agent on an execution resource.
  An execution agent may be *lightweight* in that its existance is only
  observable while the instruction stream is running.
  As such a lightweight execution agent may come into existence when
  the instruction stream starts running and cease to exist when the
  instruction stream ends.

**Execution Context:**
  The mapping of execution agents to execution resources.

**Execution Function:**
  The binding of an instruction stream to one or more execution agents.
  The instruction stream of a parallel algorithm may be bound to multiple
  execution agents that can run concurrently on an execution resource.
  An instruction stream's entry and return interface conforms to a
  specification defined by an execution function.
  An execution function targets a specific execution architecture.

**Executor:**
  Provides execution functions for running instruction streams on
  an particular, observeable execution resource.
  A particular executor targets a particular execution architecture.

# Minimal executor category

## Executor type traits

### Checking that a type is a `OneWayExecutor`

    template<class T> struct is_one_way_executor : see-below;

    template<class T> constexpr bool is_one_way_executor_v = is_one_way_executor<T>::value;

`is_one_way_executor<T>` publicly inherits from `std::true_type` if `T` satisfies the `OneWayExecutor` requirements (see Table \ref{one_way_executor_requirements}); otherwise, it publicly inherits from `std::false_type`.

### Checking that a type is a `TwoWayExecutor`

    template<class T> struct is_two_way_executor : see-below;

    template<class T> constexpr bool is_two_way_executor_v = is_two_way_executor<T>::value;

`is_two_way_executor<T>` publicly inherits from `std::true_type` if `T` satisfies the `TwoWayExecutor` requirements (see Table \ref{two_way_executor_requirements}); otherwise, it publicly inherits from `std::false_type`.

### Associated future type

    template<class Executor, class T>
    struct executor_future
    {
      private:
        template<class U>
        using helper = typename U::template future<T>;

      public:
        using type = std::experimental::detected_or_t<std::future<T>, helper, Executor, T>;

        // XXX a future proposal can relax this to enable user-defined future types 
        static_assert(std::is_same_v<type, std::future<T>>,
          "Executor-specific future types must be std::future for the minimal proposal");
    };
    
    template<class Executor, class T>
    using executor_future_t = typename executor_future<Executor,T>::type;

### Associated execution context type

    template<class Executor>
    struct executor_context
    {
      using type = std::decay_t<decltype(declref<const Executor&>().context())>; // TODO check this
    };

    template <class Executor>
    using executor_context_t = typename executor_context<Executor>::type;

## `ExecutionContext`

1.  A type meets the `ExecutionContext` requirements if it satisfies the `EqualityComparable` requirements (C++Std [equalitycomparable]). No comparison operator on these types shall exit via an exception.

## `BaseExecutor`

1. A type `X` meets the `BaseExecutor` requirements if it satisfies the requirements of `CopyConstructible` (C++Std [copyconstructible]), `Destructible` (C++Std [destructible]), and `EqualityComparable` (C++Std [equalitycomparable]), as well as the additional requirements listed below.

2. No comparison operator, copy operation, move operation, swap operation, or member function `context` on these types shall exit via an exception.

3. The executor copy constructor, comparison operators, and other member functions defined in these requirements shall not introduce data races as a result of concurrent calls to those functions from different threads.

4. The destructor shall not block pending completion of the submitted function objects. *[Note:* The ability to wait for completion of submitted function objects may be provided by the associated execution context. *--end note]*

5. In the table below, `x1` and `x2` denote (possibly const) values of type `X`, `mx1` denotes an xvalue of type `X`, and `u` denotes an identifier.

| Expression | Type | Assertion/note/pre-/post-condition |
|------------|------|------------------------------------|
| `X u(x1);` | | Shall not exit via an exception.<br/><br/>*Post:* `u == x1` and `u.context() == x1.context()`. |
| `X u(mx1);` | | Shall not exit via an exception.<br/><br/>*Post:* `u` equals the prior value of `mx1` and `u.context()` equals the prior value of `mx1.context()`. |
| `x1 == x2` | `bool` | Returns `true` only if `x1` and `x2` can be interchanged with identical effects in any of the expressions defined in these type requirements (TODO and the other executor requirements defined in this Technical Specification). *[Note:* Returning `false` does not necessarily imply that the effects are not identical. *--end note]* `operator==` shall be reflexive, symmetric, and transitive, and shall not exit via an exception. |
| `x1 != x2` | `bool` | Same as `!(x1 == x2)`. |
| `x1.context()` | `E&` or `const E&` where `E` is a type that satisfies the `ExecutionContext` requirements. | Shall not exit via an exception. The comparison operators and member functions defined in these requirements (TODO and the other executor requirements defined in this Technical Specification) shall not alter the reference returned by this function. |

## `OneWayExecutor`

1. The `OneWayExecutor` requirements form the basis of the one-way executor concept taxonomy;
   every weak one-way executor satisfies the `OneWayExecutor` requirements. This set of requirements
   specifies operations for creating execution agents that need not synchronize with the thread
   which created them.

2. No constructor, comparison operator, copy operation, move operation, or swap operation on these types shall exit via an exception.

3. In Table \ref{one_way_executor_requirements}, `f`, denotes a `MoveConstructible` function object, `a...`
   denotes a variadic argument pack of move constructible arguments, and `x` denotes an object of type `X`.

4. The executor copy constructor, comparison operators, and other member
  functions defined in these requirements shall not introduce data races as a
  result of concurrent calls to those functions from different threads.

5. A type `X` satisfies the `OneWayExecutor` requirements if:
  * `X` satisfies the `BaseExecutor` requirements.
  * For any `f`, `a...` and `x`, the expressions in Table \ref{one_way_executor_requirements} are valid and have the indicated semantics.

Table: (One-Way Executor requirements) \label{one_way_executor_requirements}

| Expression                                                                         | Return Type                                                   | Operational semantics                                                    | Assertion/note/pre-/post-condition                                 |
|------------------------------------------------------------------------------------|---------------------------------------------------------------|--------------------------------------------------------------------------|--------------------------------------------------------------------|
| `x.execute(std::move(f), std::move(a)...)`                                         |                                                               |  Creates a weakly parallel execution agent which invokes `f(a...)`       | May prevent forward progress of caller pending completion of `f.   |

## `HostBasedOneWayExecutor`

1. The `HostBasedOneWayExecutor` requirements form the basis of host-based executors in the one-way executor concept taxonomy;
   every host-based one-way executor satisfies the `HostBasedOneWayExecutor` requirements. This set of requirements
   specifies operations for creating execution agents that need not synchronize with the thread
   which created them.

2. In Table \ref{host_one_way_executor_requirements}, `f`, denotes a `MoveConstructible` function object, `a...`
   denotes a variadic argument pack of move constructible arguments, `x` denotes an object of type `X`,
   `alloc_arg` denotes an object of type `std::allocator_arg_t`, and `alloc` denotes an object satisfying
   the `ProtoAllocator` requirements.

3. A type `X` satisfies the `HostBasedOneWayExecutor` requirements if:
  * `X` satisfies the `OneWayExecutor` requirements.
  * For any `f`, `a`, `alloc`, `alloc_arg`, and `x`, the expressions in Table \ref{host_one_way_executor_requirements} are valid and have the indicated semantics.

Table: (Host-Based One-Way Executor requirements) \label{host_one_way_executor_requirements}

| Expression                                                                         | Return Type                                                   | Operational semantics                                                    | Assertion/note/pre-/post-condition                                 |
|------------------------------------------------------------------------------------|---------------------------------------------------------------|--------------------------------------------------------------------------|--------------------------------------------------------------------|
| `x.execute(std::move(f), std::move(a)...)`                                         |                                                               |  Creates a parallel execution agent which invokes `f(a...)`              | May prevent forward progress of caller pending completion of `f.   |
| `x.execute(alloc_arg, alloc, std::move(f), std::move(a)...)`                       |                                                               |  Creates a parallel execution agent which invokes `f(a...)`              | May prevent forward progress of caller pending completion of `f`.  |

## `EventExecutor`

1. The `EventExecutor` requirements defines executors for one-way event-driven execution.
   Every event executor satisfies the `EventExecutor` requirements. This set of requirements
   specifies operations for creating execution agents that need not synchronize with the thread
   which created them.

2. In Table \ref{event_executor_requirements}, `f`, denotes a `MoveConstructible` function object, `a...`
   denotes a variadic argument pack of move constructible arguments, `x` denotes an object of type `X`,
   `alloc_arg` denotes an object of type `std::allocator_arg_t`, and `alloc` denotes an object satisfying
   the `ProtoAllocator` requirements.

3. A type `X` satisfies the `EventExecutor` requirements if:
  * `X` satisfies the `HostBasedOneWayExecutor` requirements.
  * For any `f`, `a`, `alloc`, `alloc_arg`, and `x`, the expressions in Table \ref{event_executor_requirements} are valid and have the indicated semantics.

Table: (Event Executor requirements) \label{event_executor_requirements}

| Expression                                                                         | Return Type                                                   | Operational semantics                                                    | Assertion/note/pre-/post-condition                                     |
|------------------------------------------------------------------------------------|---------------------------------------------------------------|--------------------------------------------------------------------------|------------------------------------------------------------------------|
| `x.post(std::move(f), std::move(a)...)`                                            |                                                               |  Creates a parallel execution agent which invokes `f(a...)`              | May not prevent forward progress of caller pending completion of `f.   |
| `x.post(alloc_arg, alloc, std::move(f), std::move(a)...)`                          |                                                               |  Creates a parallel execution agent which invokes `f(a...)`              | May not prevent forward progress of caller pending completion of `f`.  |
| `x.defer(std::move(f), std::move(a)...)`                                           |                                                               |  Creates a parallel execution agent which invokes `f(a...)`              | May not prevent forward progress of caller pending completion of `f.   |
| `x.defer(alloc_arg, alloc, std::move(f), std::move(a)...)`                         |                                                               |  Creates a parallel execution agent which invokes `f(a...)`              | May not prevent forward progress of caller pending completion of `f`.  |

## `TwoWayExecutor`

1. The `TwoWayExecutor` requirements form the basis of the two-way executor concept taxonomy;
   every two-way executor satisfies the `TwoWayExecutor` requirements. This set of requirements
   specifies operations for creating execution agents that synchronize with the thread
   which created them.

2. In Table \ref{two_way_executor_requirements}, `f`, denotes a `MoveConstructible` function object with zero arguments whose result type is `R`,
   and `x` denotes an object of type `X`.

3. A type `X` satisfies the `TwoWayExecutor` requirements if:
  * `X` satisfies the `BaseExecutor` requirements.
  * For any `f` and `x`, the expressions in Table \ref{two_way_executor_requirements} are valid and have the indicated semantics.

Table: (Two-Way Executor requirements) \label{two_way_executor_requirements}

| Expression                                                                         | Return Type                                                   | Operational semantics                                                    | Assertion/note/pre-/post-condition                                 |
|------------------------------------------------------------------------------------|---------------------------------------------------------------|--------------------------------------------------------------------------|--------------------------------------------------------------------|
| `x.async_-` `execute(std::move(f))`                                                | `executor_-` `future_t<X,R>`                                  |  Creates an execution agent which invokes `f()`                          |                                                                    |
|                                                                                    |                                                               |  Returns the result of `f()` via the resulting future object             |                                                                    |
|                                                                                    |                                                               |  Returns any exception thrown by `f()` via the resulting future object   |                                                                    |

# Bulk (Parallelism TS) executor category

## Bulk executor traits

### Classifying forward progress guarantees of groups of execution agents

    struct sequenced_execution_tag {};
    struct parallel_execution_tag {};
    struct unsequenced_execution_tag {};

    // TODO a future proposal can define this category
    // struct concurrent_execution_tag {};

    template<class Executor>
    struct executor_execution_category
    {
      private:
        // exposition only
        template<class T>
        using helper = typename T::execution_category;

      public:
        using type = std::experimental::detected_or_t<
          unsequenced_execution_tag, helper, Executor
        >;
    };

    template<class Executor>
    using executor_execution_category_t = typename executor_execution_category<Executor>::type;

XXX TODO the relative "strength" of these categories should be defined

### Associated shape type

    template<class Executor>
    struct executor_shape
    {
      private:
        // exposition only
        template<class T>
        using helper = typename T::shape_type;
    
      public:
        using type = std::experimental::detected_or_t<
          size_t, helper, Executor
        >;

        // exposition only
        static_assert(std::is_integral_v<type>, "shape type must be an integral type");
    };

    template<class Executor>
    using executor_shape_t = typename executor_shape<Executor>::type;

### Associated index type

    template<class Executor>
    struct executor_index
    {
      private:
        // exposition only
        template<class T>
        using helper = typename T::index_type;

      public:
        using type = std::experimental::detected_or_t<
          executor_shape_t<Executor>, helper, Executor
        >;

        // exposition only
        static_assert(std::is_integral_v<type>, "index type must be an integral type");
    };

    template<class Executor>
    using executor_index_t = typename executor_index<Executor>::type;

## `BulkTwoWayExecutor`

1. The `BulkTwoWayExecutor` requirements form the basis of the bulk executor concept.
   This set of requirements specifies operations for creating groups of execution agents in bulk from a single operation
   with the ability to synchronize these groups of agents with another thread.

2. In Table \ref{bulk_two_way_executor_requirements},
    * `f` denotes a `CopyConstructible` function object with three arguments,
    * `n` denotes a shape object whose type is `executor_shape_t<X>`.
    * `rf` denotes a `CopyConstructible` function object with one argument whose result type is `R`,
    * `sf` denotes a `CopyConstructible` function object with one argument whose result type is `S`,
    * `i` denotes an object whose type is `executor_index_t<X>`,
    * `r` denotes an object whose type is `R`, 
    * `s` denotes an object whose type is `S`, and
    * `pred` denotes a future object whose result is `pr`.

2. A class `X` satisfies the requirements of a bulk executor if `X` satisfies
   either the `OneWayExecutor` or `TwoWayExecutor` requirements and the expressions of Table
   \ref{bulk_two_way_executor_requirements} are valid and have the indicated semantics.

Table: (Bulk two-way executor requirements) \label{bulk_two_way_executor_requirements}

| Expression                                                        | Return Type                                                       |  Operational semantics                                                                                     | Assertion/note/pre-/post-condition                                                                                                                         |
|-------------------------------------------------------------------|-------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `x.bulk_sync_-` `execute(f, n, rf, sf)`                           | `R`                                                               |  Creates a group of execution agents of shape `n` which invoke `f(i, r, s)`                                | Note: blocks the forward progress of the caller until all invocations of `f` are finished.                                                                 |
|                                                                   |                                                                   |  Returns the result of `rf(n)`                                                                             | Effects: invokes `rf(n)` on an unspecified execution agent.                                                                                                |
|                                                                   |                                                                   |                                                                                                            | Effects: invokes `sf(n)` on an unspecified execution agent.                                                                                                |
|                                                                   |                                                                   |                                                                                                            |                                                                                                                                                            |
| `x.bulk_async_-` `execute(f, n, rf, sf)`                          | `executor_-` `future_t<X,R>`                                      |  Creates a group of execution agents of shape `n` which invoke `f(i, r, s)`                                | Effects: invokes `rf(n)` on an unspecified execution agent.                                                                                                |
|                                                                   |                                                                   |  Asynchronously returns the result of `rf(n)` via the resulting future object                              | Effects: invokes `sf(n)` on an unspecified execution agent.                                                                                                |
|                                                                   |                                                                   |                                                                                                            |                                                                                                                                                            |
|                                                                   |                                                                   |                                                                                                            |                                                                                                                                                            |
| `x.bulk_then_-` `execute(f, n, rf, pred, sf)`                     | `executor_-` `future_t<X,R>`                                      |  Creates a group of execution agents of shape `n` which invoke `f(i, r, pr, s)` after `pred` becomes ready | Effects: invokes `rf(n)` on an unspecified execution agent.                                                                                                |
|                                                                   |                                                                   |  Asynchronously returns the result of `rf(n)` via the resulting future.                                    | Effects: invokes `sf(n)` on an unspecified execution agent.                                                                                                |
|                                                                   |                                                                   |                                                                                                            | If `pred`'s result type is `void`, `pr` is omitted from `f`'s invocation.                                                                                  |
|                                                                   |                                                                   |                                                                                                            | Post: `pred` is invalid if it is not a shared future.                                                                                                      |


XXX TODO: need to specify how `executor_execution_category_t` describes the forward progress requirements of a group of execution agents wrt each other

## `ExecutorWorkTracker`

1. The `ExecutorWorkTracker` requirements defines operations for tracking future work against an executor.

2. A type `X` meets the `ExecutorWorkTracker` requirements if it satisfies the requirements of `CopyConstructible` (C++Std [copyconstructible]) and `Destructible` (C++Std [destructible]), as well as the additional requirements listed below.

3. No constructor, comparison operator, copy operation, move operation, swap operation, or member functions `on_work_started` and `on_work_finished` on these types shall exit via an exception.

4. The executor copy constructor, comparison operators, and other member functions defined in these requirements shall not introduce data races as a result of concurrent calls to those functions from different threads.

5. In Table \ref{executor_work_tracker_requirements}, `x` denotes an object of type `X`,

Table: (Executor Work Tracker requirements) \label{executor_work_tracker_requirements}

| Expression | Return Type | Assertion/note/pre-/post-condition |
|------------|-------------|------------------------------------|
| `x.on_work_started()` | `bool` | Shall not exit via an exception. |
| `x.on_work_finished()` | | Shall not exit via an exception. Precondition: A corresponding preceding call to `on_work_started` that returned `true`. |

# (Networking TS) executor category

XXX TODO

# Executor Customization Points

## In general

1. The functions described in this clause are *executor customization points*.
   Executor customization points provide a uniform interface to all executor types.

2. An executor customization point does not participate in overload resolution
   if its `exec` parameter is neither a `OneWayExecutor` nor a `TwoWayExecutor`. Executor customization points
   follow the design suggested by [N4381](https://wg21.link/N4381) and [P0285](https://wg21.link/P0285).

   XXX the reason p2 is included is to define some general purpose wording for executor customization points in order to avoid repetition below.
       but, there's still some repetition

## Function template `execution::execute()`

1.  ```
    template<class Executor, class Function>
    void execute(Executor& exec, Function&& f);
    ```

2. *Effects:* calls `exec.execute(std::forward<Function>(f))` if that call is well-formed; otherwise, calls
   `DECAY_COPY(std::forward<Function>(f))()` in a new execution agent with the call to `DECAY_COPY()` being evaluated
   in the thread that called `execute`. Any return value is discarded.

## Function template `execution::sync_execute()`

1.  ```
    template<class Executor, class Function>
    result_of_t<decay_t<Function>()>
    sync_execute(Executor& exec, Function&& f);
    ```

2. *Effects:* calls `exec.sync_execute(std::forward<Function>(f))` if that call is well-formed; otherwise, calls
   `DECAY_COPY(std::forward<Function>(f))()` in a new execution agent with the call to `DECAY_COPY()` being
   evaluated in the thread that called `execute`.

3. *Returns:* The return value of `f`.

4. *Synchronization:* The invocation of `sync_execute` synchronizes with (1.10) the invocation of `f`.

5. *Throws:* Any uncaught exception thrown by `f`.

## Function template `execution::async_execute()`

1.  ```
    template<class Executor, class Function>
    executor_future_t<
      Executor,
      result_of_t<decay_t<Function>()>
    >
    async_execute(Executor& exec, Function&& f);
    ```

2. *Effects:* calls `exec.async_execute(std::forward<Function>(f))` if that call is well-formed;

    otherwise, calls `DECAY_COPY(std::forward<Function>(f))()` in a new execution agent with the call to `DECAY_COPY()` being
    evaluated in the thread that called `async_execute`. Any return value is stored as the result in the shared
    state. Any exception propagated from the execution of `INVOKE(DECAY_COPY(std::forward<Function>(f))` is stored as
    the exceptional result in the shared state.

3. *Returns:* An object of type

    `executor_future_t<Executor,result_of_t<decay_t<Function>()>>`
   
    that refers to the shared state created by this call to `async_execute`.

4. *Synchronization:*
    * the invocation of `async_execute` synchronizes with (1.10) the invocation of `f`.
    * the completion of the function `f` is sequenced before (1.10) the shared state is made ready.

## Function template `execution::then_execute()`

1.  ```
    template<class Executor, class Function, class Future>
    executor_future_t<Executor,see-below>
    then_execute(Executor& exec, Function&& f, Future& predecessor);
    ```

2. *Effects:* calls

    `exec.then_execute(std::forward<Function>(f), std::forward<Future>(predecessor))`
    
    if that call is well-formed; otherwise:

    * Creates a shared state that is associated with the returned future object.

    * Creates a new execution agent `predecessor` becomes ready. The execution agent calls `f(pred)`, where `pred` is a reference to
      the `predecessor` state if it is not `void`. Otherwise, the execution agent calls `f()`.

    * Any return value of `f` is stored as the result in the shared state of the resulting future.

    * Any exception thrown by `f` is stored as the exceptional result in the shared state of the resulting future.

3. *Returns:* `executor_future_t<Executor,result_of_t<decay_t<Function>()>` when `predecessor` is a `void` future. Otherwise,
   `executor_future_t<Executor,result_of_t<decay_t<Function>(T&)>>` where `T` is the result type of the `predecessor` future.

4. *Synchronization:*
    * the invocation of `then_execute` synchronizes with (1.10) the invocation of `f`.
    * the completion of the invocation of `f` is sequenced before (1.10) the shared state is made ready.

5. *Postconditions:* If the `predecessor` future is not a shared future, then `predecessor.valid() == false`.


## Function template `execution::bulk_execute()`

1.  ```
    template<class Executor, class Function1, class Function2>
    void bulk_execute(Executor& exec, Function1 f, executor_shape_t<Executor> shape,
                      Function2 shared_factory);
    ```

2. *Effects:* calls `exec.bulk_execute(f, shape, shared_factory)` if that call is well-formed;
   otherwise:

   * Calls `shared_factory(shape)` in an unspecified execution agent. The result of this invocation is stored to shared state.

   * Creates a new group of execution agents of shape `shape`. Each execution agent calls `f(idx, shared)`, where
     `idx` is the index of the execution agent, and `shared` is a reference to the shared state.

   * If any invocation of `f` exists via an uncaught exception, `terminate` is called.

3. *Synchronization:* The completion of the function `shared_factory` happens before the creation of the group of execution agents.


## Function template `execution::bulk_sync_execute()`

1.  ```
    template<class Executor, class Function1, class Function2, class Function3>
    result_of_t<Function2(executor_shape_t<Executor>)>
    bulk_sync_execute(Executor& exec, Function1 f, executor_shape_t<Executor> shape,
                      Function2 result_factory, Function3 shared_factory);
    ```

2. *Effects:* calls `exec.bulk_sync_execute(f, shape, result_factory, shared_factory)` if that call is well-formed;
   otherwise:
   
   * Calls `result_factory(shape)` and `shared_factory(shape)` in an unspecified execution agent. The results
     of these invocations are stored to shared state.

   * Creates a new group of execution agents of shape `shape`. Each execution agent calls `f(idx, result, shared)`, where
     `idx` is the index of the execution agent, and `result` and `shared` are references to the respective shared state.
     Any return value of `f` is discarded.

   * If any invocation of `f` exits via an uncaught exception, `terminate` is called.

3. *Returns:* An object of type `result_of_t<Function2(executor_shape_t<Executor>)>` that refers to the result shared state created by
   this call to `bulk_sync_execute`.

4. *Synchronization:* The completion of the functions `result_factory` and
   `shared_factory` happen before the creation of the group of execution
   agents.

## Function template `execution::bulk_async_execute()`

1.  ```
    template<class Executor, class Function1, class Function2, class Function3>
    executor_future_t<
      Executor,
      result_of_t<Function2(executor_shape_t<Executor>)>
    >
    bulk_async_execute(Executor& exec, Function1 f, executor_shape_t<Executor> shape,
                       Function2 result_factory, Function3 shared_factory);
    ```

2. *Effects:* calls `exec.bulk_async_execute(f, shape, result_factory, shared_factory)` if that call is well-formed;
   otherwise:

    * Calls `result_factory(shape)` and `shared_factory(shape)` in an unspecified execution agent. The results of these
      invocations are stored to shared state.

    * Creates a new group of execution agents of shape `shape`. Each execution agent calls `f(idx, result, shared)`, where
      `idx` is the index of the execution agent, and `result` and `shared` are references to the respective shared state.
      Any return value of `f` is discarded.

   * If any invocation of `f` exits via an uncaught exception, `terminate` is called.

3. *Returns:* An object of type

    `executor_future_t<Executor,result_of_t<Function2(executor_shape_t<Executor>)>`
    
    that refers to the shared result state created by this call to `bulk_async_execute`.

4. *Synchronization:*
    * the invocation of `bulk_async_execute` synchronizes with (1.10) the invocations of `f`.
    * the completion of the functions `result_factory` and `shared_factory` happen before the creation of the group of execution agents.
    * the completion of the invocations of `f` are sequenced before (1.10) the result shared state is made ready.

## Function template `execution::bulk_then_execute()`

1.  ```
    template<class Executor, class Function1, class Future, class Function2, class Function3>
    executor_future_t<
      Executor,
      result_of_t<Function2(executor_shape_t<Executor>)>
    >
    bulk_then_execute(Executor& exec, Function1 f, executor_shape_t<Executor> shape,
                      Future& predecessor,
                      Function2 result_factory, Function3 shared_factory);
    ```

2. *Effects:* calls `exec.bulk_then_execute(f, shape, predecessor, result_factory, shared_factory)` if that call is well-formed;
   otherwise:

   * Calls `result_factory(shape)` and `shared_factory(shape)` in an unspecified execution agent. The results of these
     invocations are stored to shared state.

   * Creates a new group of execution agents of shape `shape` after `predecessor` becomes ready. Each execution agent calls `f(idx, result, pred, shared)`, where
     `idx` is the index of the execution agent, `result` is a reference to the result shared state, `pred` is a reference to
     the `predecessor` state if it is not `void`. Otherwise, each execution agent calls `f(idx, result, shared)`.
     Any return value of `f` is discarded.

   * If any invocation of `f` exits via an uncaught exception, `terminate` is called.

3. *Returns:* An object of type

    `executor_future_t<Executor,result_of_t<Function2(executor_shape_t<Executor>)>`
    
    that refers to the shared result state created by this call to `bulk_then_execute`.

4. *Synchronization:*
    * the invocation of `bulk_then_execute` synchronizes with (1.10) the invocations of `f`.
    * the completion of the functions `result_factory` and `shared_factory` happen before the creation of the group of execution agents.
    * the completion of the invocations of `f` are sequenced before (1.10) the result shared state is made ready.

5. *Postconditions:* If the `predecessor` future is not a shared future, then `predecessor.valid() == false`.

# Execution policy interoperation

```
class parallel_execution_policy
{
  public:
    // types:
    using execution_category = parallel_execution_tag;
    using executor_type = implementation-defined;

    // executor access
    const executor_type& executor() const noexcept;

    // execution policy factory
    template<class Executor>
    see-below on(Executor&& exec) const;
};

class sequenced_execution_tag { by-analogy-to-parallel_execution_policy };
class parallel_unsequenced_execution_tag { by-analogy-to-parallel_execution_policy };
```

## Associated executor

1. Each execution policy is associated with an executor, and this executor is called its *associated executor*.

2. The type of an execution policy's associated executor shall satisfy the requirements of `BulkTwoWayExecutor`.

3. When an execution policy is used as a parameter to a parallel algorithm, the
   execution agents that invoke element access functions are created by the
   execution policy's associated executor.

4. The type of an execution policy's associated executor is the same as the member type `executor_type`.

## Execution category

1. Each execution policy is categorized by an *execution category*.

2. When an execution policy is used as a parameter to a parallel algorithm, the
   execution agents it creates are guaranteed to make forward progress and
   execute invocations of element access functions as ordered by its execution
   category.

3. An execution policy's execution category is given by the member type `execution_category`.

4. The execution category of an execution policy's associated executor shall not be weaker than the execution policy's execution category.

## Associated executor access

1.  ```
    const executor_type& executor() const noexcept;
    ```

2. *Returns:* The execution policy's associated executor.

## Execution policy factory

1.  ```
    template<class Executor>
    see-below on(Executor&& exec) const;
    ```

2. Let `T` be `decay_t<Executor>`.

3. *Returns:* An execution policy whose execution category is `execution_category`. If `T` satisfies the requirements of
   `BulkTwoWayExecutor`, the returned execution policy's associated executor is equal to `exec`. Otherwise,
   the returned execution policy's associated executor is an adaptation of `exec`.

   XXX TODO: need to define what adaptation means

3. *Remarks:* This member function shall not participate in overload resolution unless `is_executor_v<T>` is `true` and
   `executor_execution_category_t<T>` is as strong as `execution_category`.

# Control structure interoperation

## Function template `async`

1. The function template `async` provides a mechanism to invoke a function in a new
   execution agent created by an executor and provides the result of the function in the
   future object with which it shares a state.

    ```
    template<class Executor, class Function, class... Args>
    executor_future_t<Executor, result_of_t<decay_t<Function>(decay_t<Args>...)>>
    async(Executor& exec, Function&& f, Args&&... args);
    ```

2. *Returns:* Equivalent to:

    `return execution::async_execute(exec, [=]{ return INVOKE(f, args...); });`

    XXX This forwarding doesn't look correct to me

## `std::future::then()`

1. The member function template `then` provides a mechanism for attaching a *continuation* to a `std::future` object,
   which will be executed on a new execution agent created by an executor.

    ```
    template<class T>
    template<class Executor, class Function>
    executor_future_t<Executor, see-below>
    future<T>::then(Executor& exec, Function&& f);
    ```

2. *Returns:* Equivalent to:

    `return execution::then_execute(exec, std::forward<Function>(f), *this);`

    XXX This forwarding doesn't look correct to me

## `std::shared_future::then()`

1. The member function template `then` provides a mechanism for attaching a *continuation* to a `std::shared_future` object,
   which will be executed on a new execution agent created by an executor.

    ```
    template<class T>
    template<class Executor, class Function>
    executor_future_t<Executor, see-below>
    shared_future<T>::then(Executor& exec, Function&& f);
    ```

2. *Returns:* Equivalent to:

    `return execution::then_execute(exec, std::forward<Function>(f), *this);`

## Function template `invoke`

1. The function template `invoke` provides a mechanism to invoke a function in a new
   execution agent created by an executor and return result of the function.

    ```
    template<class Executor, class Function, class... Args>
    result_of_t<F&&(Args&&...)>
    invoke(Executor& exec, Function&& f, Args&&... args);
    ```

2. *Returns:* Equivalent to:

    `return execution::sync_execute(exec, [&]{ return INVOKE(f, args...); });`

## Task block

### Function template `define_task_block_restore_thread()`

1.  ```
    template<class Executor, class F>
    void define_task_block_restore_thread(Executor& exec, F&& f);
    ```

2. *Requires:* Given an lvalue `tb` of type `task_block`, the expression `f(tb)` shall be well-formed.

3. *Effects:* Constructs a `task_block tb`, creates a new execution agent, and calls `f(tb)` on that execution agent.

4. *Throws:* `exception_list`, as specified in version two of the Paralellism TS.

5. *Postconditions:* All tasks spawned from `f` have finished execution.

6. *Remarks:* Unlike `define_task_block`, `define_task_block_restore_thread` always returns on the same thread as the one on which it was called.

### `task_block` member function template `run`

1.  ```
    template<class Executor, class F>
    void run(Executor& exec, F&& f);
    ```

2. *Requires:* `F` shall be `MoveConstructible`. `DECAY_COPY(std::forward<F>(f))()` shall be a valid expression.

3. *Preconditions:* `*this` shall be an active `task_block`.

4. *Effects:* Evaluates `DECAY_COPY(std::forward<F>(f))()`, where `DECAY_COPY(std::forward<F>(f))` is evaluated synchronously within the current thread.
   The call to the resulting copy of the function object is permitted to run on an execution agent created by `exec` in an unordered fashion relative to
   the sequence of operations following the call to `run(exec, f)` (the continuation), or indeterminately-sequenced within the same thread as the continuation.
   The call to `run` synchronizes with the next invocation of `wait` on the same `task_block` or completion of the nearest enclosing `task_block` (i.e., the `define_task_block` or
   `define_task_block_restore_thread` that created this `task_block`.

5. *Throws:* `task_cancelled_exception`, as described in version 2 of the Parallelism TS.

# Executor work guard

```
template<class Executor>
class executor_work_guard
{
public:
  // types:

  typedef Executor executor_type;

  // construct / copy / destroy:

  explicit executor_work_guard(const executor_type& ex) noexcept;
  executor_work_guard(const executor_work_guard& other) noexcept;
  executor_work_guard(executor_work_guard&& other) noexcept;

  executor_work_guard& operator=(const executor_work_guard&) = delete;

  ~executor_work_guard();

  // executor work guard observers:

  executor_type get_executor() const noexcept;
  bool owns_work() const noexcept;

  // executor work guard modifiers:

  void reset() noexcept;

private:
  Executor ex_; // exposition only
  bool owns_; // exposition only
};
```

## Members

```
explicit executor_work_guard(const executor_type& ex) noexcept;
```

*Effects:* Initializes `ex_` with `ex`, and `owns_` with the result of
`ex_.on_work_started()`.

*Postconditions:* `ex_ == ex`.

```
executor_work_guard(const executor_work_guard& other) noexcept;
```

*Effects:* Initializes `ex_` with `other.ex_`. If `other.owns_ == true`,
initializes `owns_` with the result of `ex_.on_work_started()`; otherwise, sets
`owns_` to false.

*Postconditions:* `ex_ == other.ex_`.

```
executor_work_guard(executor_work_guard&& other) noexcept;
```

*Effects:* Initializes `ex_` with `std::move(other.ex_)` and `owns_` with
`other.owns_`, and sets `other.owns_` to `false`.

```
~executor_work_guard();
```

*Effects:* If `owns_` is `true`, performs `ex_.on_work_finished()`.

```
executor_type get_executor() const noexcept;
```

*Returns:* `ex_`.

```
bool owns_work() const noexcept;
```

*Returns:* `owns_`.

```
void reset() noexcept;
```

*Effects:* If `owns_` is `true`, performs `ex_.on_work_finished()`.

*Postconditions:* `owns_ == false`.

# Polymorphic executor wrappers

## Class `one_way_executor`

XXX this section has a lot of wording redundant with `any`. it would be nice if the constructors & 
    modifiers effects/return clauses could say something like "as if by corresponding-expression-involving-some-expository-`std::any`-object"


```
class one_way_executor
{
  public:
    // construction and destruction
  
    // XXX a future proposal can introduce the default constructor
    //     and account for empty states
    // one_way_executor() noexcept;

    one_way_executor(const one_way_executor& other);

    // XXX this proposal omits move construction because
    //     it seems like it would leave the other executor
    //     in an empty state
    // one_way_executor(one_way_executor&& other) noexcept;

    template<class OneWayExecutor>
    one_way_executor(OneWayExecutor&& exec);

    // assignments
    one_way_executor& operator=(const one_way_executor& rhs);

    template<class OneWayExecutor>
    one_way_executor& operator=(exec& rhs);

    // modifiers
    void swap(one_way_executor& rhs) noexcept;

    // execution agent creation
    template<class Function>
    void execute(Function&& f);

    // XXX future proposals can introduce member functions for each
    //     Executor customization point

  private:
    std::any contained_executor_; // exposition only
};

// non-member functions
void swap(one_way_executor& x, one_way_executor& y);

bool operator==(const one_way_executor& x, const one_way_executor& y);
```

1. An object of class `one_way_executor` stores an instance of any `OneWayExecutor` type. The stored instance is called the *contained executor*.

### Construction and destruction

1.  ```
    one_way_executor(const one_way_executor& other);
    ```

2. *Effects:* Constructs an object of type `one_ay_executor` with a copy of `other`'s contained executor.

3. *Throws:* Any exceptions arising from calling the selected constructor of the contained executor.


4.  ```
    one_way_executor(one_way_executor&& other);
    ```

5. *Effects:* Constructs an object of type `one_way_executor` with a contained executor move constructed from `other`'s contained executor.

6.  ```
    template<class OneWayExecutor>
    one_way_executor(OneWayExecutor&& exec);
    ```

7. Let `T` be `decay_t<OneWayExecutor>`.

8. *Effects:* Constructs an object of type `one_way_executor` that contains an executor of type `T` direct-initialized with
   `std::forward<OneWayExecutor>(exec)`.

9. *Remarks:* This constructor shall not participate in overload resolution unless `is_one_way_executor_v<T>` is `true` or if `T` is the same type as `one_way_executor`.

10. *Throws:* Any exception thrown by the selected constructor of `T`.

### Assignment

1.  ```
    one_way_executor& operator=(const one_way_executor& rhs);
    ```

2. *Effects:* As if by `one_way_executor(rhs).swap(*this)`. No effects if an exception is thrown.

3. *Returns:* `*this`.

4. *Throws:* Any exceptions arising from the copy constructor of the contained executor.

5.  ```
    template<class OneWayExecutor>
    one_way_executor& operator=(OneWayExecutor&& rhs);
    ```

6. Let `T` be `decay_t<OneWayExecutor>`.

7. *Effects:* Constructs an object `tmp` of type `one_way_executor` that contains an executor of type `T` direct-initialized with
   `std::forward<OneWayExecutor>(rhs)`, and `tmp.swap(*this)`. No effects if an exception is thrown.

7. *Remarks*: This operator does not participate in overload resolution unless `is_one_way_executor_v<T>` is `true` or if `T` is the same type as `one_way_executor`.

8. *Throws:* Any exception thrown by the selected constructor of `T`.

### Modifiers

1.  ```
    void swap(one_way_executor& rhs) noexcept;
    ```

2. *Effects:* Exchanges the contained executors of `*this` and `rhs`.

### Execution agent creation

#### Member function `execute`

1.  ```
    template<class Function>
    void execute(Function&& f);
    ```

2. Let `exec` be this `one_way_executor`'s contained executor.

3. *Effects:* As if by `execution::execute(exec, std::forward<Function>(f))`.

### Non-member functions

1.  ```
    void swap(one_way_executor& x, one_way_executor& y) noexcept;
    ```

2. *Effects:* As if by `x.swap(y)`.


3.  ```
    bool operator==(const one_way_executor& x, const one_way_executor& y)
    ```

4. Let `x_exec` be `x`'s contained executor and `y_exec` be `y`'s contained executor.

4. *Returns:* `x_exec == y_exec`.


## Class `two_way_executor`

XXX this section has a lot of wording redundant with `any`. it would be nice if the constructors & 
    modifiers effects/return clauses could say something like "as if by corresponding-expression-involving-some-expository-`std::any`-object"


```
class two_way_executor
{
  public:
    // construction and destruction
  
    // XXX a future proposal can introduce the default constructor
    //     and account for empty states
    // two_way_executor() noexcept;

    two_way_executor(const two_way_executor& other);

    // XXX this proposal omits move construction because
    //     it seems like it would leave the other executor
    //     in an empty state
    // two_way_executor(two_way_executor&& other) noexcept;

    template<class TwoWayExecutor>
    two_way_executor(TwoWayExecutor&& exec);

    // assignments
    two_way_executor& operator=(const two_way_executor& rhs);

    template<class OneWayExecutor>
    two_way_executor& operator=(exec& rhs);

    // modifiers
    void swap(two_way_executor& rhs) noexcept;

    // execution agent creation
    template<class Function>
    future<result_of_t<decay_t<Function>()>>
    async_execute(Function&& f);

    // XXX future proposals can introduce member functions for each
    //     Executor customization point

  private:
    std::any contained_executor_; // exposition only
};

// non-member functions
void swap(two_way_executor& x, two_way_executor& y);

bool operator==(const two_way_executor& x, const two_way_executor& y);
```

1. An object of class `two_way_executor` stores an instance of any `TwoWayExecutor` type. The stored instance is called the *contained executor*.

### Construction and destruction

1.  ```
    two_way_executor(const two_way_executor& other);
    ```

2. *Effects:* Constructs an object of type `one_way_executor` with a copy of `other`'s contained executor.

3. *Throws:* Any exceptions arising from calling the selected constructor of the contained executor.


4.  ```
    two_way_executor(two_way_executor&& other);
    ```

5. *Effects:* Constructs an object of type `two_way_executor` with a contained executor move constructed from `other`'s contained executor.

6.  ```
    template<class OneWayExecutor>
    two_way_executor(OneWayExecutor&& exec);
    ```

7. Let `T` be `decay_t<OneWayExecutor>`.

8. *Effects:* Constructs an object of type `two_way_executor` that contains an executor of type `T` direct-initialized with
   `std::forward<OneWayExecutor>(exec)`.

9. *Remarks:* This constructor shall not participate in overload resolution unless `is_two_way_executor_v<T>` is `true` or if `T` is the same type as `two_way_executor`.

10. *Throws:* Any exception thrown by the selected constructor of `T`.

### Assignment

1.  ```
    two_way_executor& operator=(const two_way_executor& rhs);
    ```

2. *Effects:* As if by `two_way_executor(rhs).swap(*this)`. No effects if an exception is thrown.

3. *Returns:* `*this`.

4. *Throws:* Any exceptions arising from the copy constructor of the contained executor.

5.  ```
    template<class OneWayExecutor>
    two_way_executor& operator=(OneWayExecutor&& rhs);
    ```

6. Let `T` be `decay_t<OneWayExecutor>`.

7. *Effects:* Constructs an object `tmp` of type `two_way_executor` that contains an executor of type `T` direct-initialized with
   `std::forward<OneWayExecutor>(rhs)`, and `tmp.swap(*this)`. No effects if an exception is thrown.

7. *Remarks*: This operator does not participate in overload resolution unless `is_two_way_executor_v<T>` is `true` or if `T` is the same type as `two_way_executor`.

8. *Throws:* Any exception thrown by the selected constructor of `T`.

### Modifiers

1.  ```
    void swap(two_way_executor& rhs) noexcept;
    ```

2. *Effects:* Exchanges the contained executors of `*this` and `rhs`.

### Execution agent creation

#### Member function `async_execute`

1.  ```
    template<class Function>
    future<result_of_t<decay_t<Function>()>>
    async_execute(Function&& f);
    ```

2. Let `exec` be this `executor`'s contained executor.

3. *Returns:* Equivalent to `execution::async_execute(exec, std::forward<Function>(f))`.

XXX This equivalent expression requires giving `std::future<T>` a constructor which would move convert from `executor_future_t<X,T>`, where `X` is the type of `exec`.

### Non-member functions

1.  ```
    void swap(two_way_executor& x, two_way_executor& y) noexcept;
    ```

2. *Effects:* As if by `x.swap(y)`.

3.  ```
    bool operator==(const two_way_executor& x, const two_way_executor& y)
    ```

4. Let `x_exec` be `x`'s contained executor and `y_exec` be `y`'s contained executor.

4. *Returns:* `x_exec == y_exec`.


# Thread pool type


XXX Consider whether we should include a wording for a concurrent executor which
would satisfy the needs of async (thread pool provides parallel execution
semantics).

## Class `thread_pool`

This class represents a statically sized thread pool as a common/basic resource
type. This pool provides an effectively unbounded input queue and as such calls
to add tasks to a thread_pool's executor will not block on the input queue.

```
class thread_pool
{
  public:
    class executor_type;
    
    // construction/destruction
    thread_pool();
    explicit thread_pool(std::size_t num_threads);
    
    // nocopy
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    // stop accepting incoming work and wait for work to drain
    ~thread_pool();

    // attach current thread to the thread pools list of worker threads
    void attach();

    // signal all work to complete
    void stop();

    // wait for all threads in the thread pool to complete
    void wait();

    // placeholder for a general approach to getting executors from 
    // standard contexts.
    executor_type executor() noexcept;
};

bool operator==(const thread_pool& a, const thread_pool& b) noexcept;
bool operator!=(const thread_pool& a, const thread_pool& b) noexcept;
```

The class `thread_pool` satisfies the `ExecutionContext` requirements.

For an object of type `thread_pool`, *outstanding work* is defined as the sum
of:

* the total number of calls to the `on_work_started` function that returned
  `true`, less the total number of calls to the `on_work_finished` function, on
  any executor associated with the `thread_pool`.

* the number of function objects that have been added to the `thread_pool`
  via the `thread_pool` executor, but not yet executed; and

* the number of function objects that are currently being executed by the
  `thread_pool`.

The `thread_pool` member functions `executor`, `attach`, `wait`, and `stop`,
and the `thread_pool::executor_type` copy constructors and member functions, do
not introduce data races as a result of concurrent calls to those functions
from different threads of execution.

### Construction and destruction

```
thread_pool();
```

*Effects:* Constructs a `thread_pool` object with an implementation defined
number of threads of execution, as if by creating objects of type `thread`.

```
thread_pool(std::size_t num_threads);
```

*Effects:* Constructs a `thread_pool` object with `num_threads` threads of
execution, as if by creating objects of type `thread`. (QUESTION: Do we want to
allow 0?)

```
~thread_pool();
```

*Effects:* Destroys an object of class `thread_pool`. Performs `stop()`
followed by `wait()`.

### Worker Management

```
void attach();
```

*Effects:* adds the calling thread to the pool of workers. Blocks the calling
thread until signalled to complete by `stop()` or `wait()`, and then blocks
until all the threads created during `thread_pool` object construction have
completed. (Note: The implementation is encouraged, but not required, to use
the attached thread to execute submitted function objects. RATIONALE:
implementations in terms of the Windows thread pool cannot utilise
user-provided threads. --end note) (NAMING: a possible alternate name for this
function is `join()`.)

```
void stop();
```

*Effects:* Signals the threads in the pool to complete as soon as possible. If
a thread is currently executing a function object, the thread will exit only
after completion of that function object. The call to `stop()` returns without
waiting for the threads to complete. Subsequent calls to attach complete
immediately.

```
void wait();
```

*Effects:* If not already stopped, signals the threads in the pool to complete
once the outstanding work is `0`. Blocks the calling thread (C++Std
[defns.block]) until all threads in the pool have completed, without executing
submitted function objects in the calling thread. Subsequent calls to attach
complete immediately.

*Synchronization:* The completion of each thread in the pool synchronizes with
(C++Std [intro.multithread]) the corresponding successful `wait()` return.

### Executor Creation

```
executor_type executor() noexcept;
```

*Returns:* An executor that may be used to submit function objects to the
thread pool.

### Comparisons

```
bool operator==(const thread_pool& a, const thread_pool& b) noexcept;
```

*Returns:* `std::addressof(a) == std::addressof(b)`.

```
bool operator!=(const thread_pool& a, const thread_pool& b) noexcept;
```

*Returns:* `!(a == b)`.

## Class `thread_pool::executor_type`

```
class thread_pool::executor_type
{
  public:
    // construct / copy / destroy:

    executor_type(const executor_type& other) noexcept;
    executor_type(executor_type&& other) noexcept;

    executor_type& operator=(const executor_type& other) noexcept;
    executor_type& operator=(executor_type&& other) noexcept;

    // executor operations:

    bool running_in_this_thread() const noexcept;

    thread_pool& context() const noexcept;

    bool on_work_started() const noexcept;
    void on_work_finished() const noexcept;

    template<class Func, class Args...>
      void execute(Func&& f, Args&&... args) const;
    template<class ProtoAllocator, class Func, class Args...>
      void execute(allocator_arg_t, const ProtoAllocator& a,
        Func&& f, Args&&... args) const;

    template<class Func, class Args...>
      void post(Func&& f, Args&&... args) const;
    template<class ProtoAllocator, class Func, class Args...>
      void post(allocator_arg_t, const ProtoAllocator& a,
        Func&& f, Args&&... args) const;

    template<class Func, class Args...>
      void defer(Func&& f, Args&&... args) const;
    template<class ProtoAllocator, class Func, class Args...>
      void defer(allocator_arg_t, const ProtoAllocator& a,
        Func&& f, Args&&... args) const;

    template <typename T>
    using future = std::future<T>;

    template<class Function>
      void sync_execute(Function&& f) const;

    template<class Function>
      future<result_of_t<decay_t<Function>()>>
        async_execute(Function&& f) const;

    // TODO: meet other requirements.
};
```

`thread_pool::executor_type` is a type satisfying the `EventExecutor` and
`TwoWayExecutor` requirements. (TODO: satisfy other requirements as well.)
Objects of type `thread_pool::executor_type` are associated with a
`thread_pool`, and function objects submitted using the `execute`, `post`,
`defer`, `sync_execute`, and `async_execute` member functions will be executed
by the `thread_pool`.

### Constructors

```
executor_type(const executor_type& other) noexcept;
```

*Postconditions:* `*this == other`.

```
executor_type(executor_type&& other) noexcept;
```

*Postconditions:* `*this` is equal to the prior value of `other`.

### Assignment

```
executor_type& operator=(const executor_type& other) noexcept;
```

*Postconditions:* `*this == other`.

*Returns:* `*this`.

```
executor_type& operator=(executor_type&& other) noexcept;
```

*Postconditions:* `*this` is equal to the prior value of `other`.

*Returns:* `*this`.

### Operations

```
bool running_in_this_thread() const noexcept;
```

*Returns:* `true` if the current thread of execution is a thread that was
created by or attached to the associated `thread_pool` object.

```
thread_pool& context() const noexcept;
```

*Returns:* A reference to the associated `thread_pool` object.

```
bool on_work_started() const noexcept;
```

*Effects:* Increments the count of outstanding work associated with the
`thread_pool`.

```
void on_work_finished() const noexcept;
```

*Effects:* Decrements the count of outstanding work associated with the
`thread_pool`.

```
template<class Func, class Args...>
  void execute(Func&& f, Args&&... args) const;
```

*Effects:* If `running_in_this_thread()` is `true`, calls
`DECAY_COPY(forward<Func>(f))(forward<Args>(args)...)`. (Note: If `f` exits via
an exception, the exception propagates to the caller of `execute`. --end note)
Otherwise, calls `post(forward<Func>(f), forward<Args>(args)...)`.

```
template<class ProtoAllocator, class Func, class Args...>
  void execute(allocator_arg_t, const ProtoAllocator& a,
    Func&& f, Args&&... args) const;
```

*Effects:* If `running_in_this_thread()` is `true`, calls
`DECAY_COPY(forward<Func>(f))(forward<Args>(args)...)`. (Note: If `f` exits via
an exception, the exception propagates to the caller of `execute`. --end note)
Otherwise, calls `post(allocator_arg, a, forward<Func>(f), forward<Args>(args)...)`.

```
template<class Func, class Args...>
  void post(Func&& f, Args&&... args) const;
template<class ProtoAllocator, class Func, class Args...>
  void post(allocator_arg_t, const ProtoAllocator& a,
    Func&& f, Args&&... args) const;
```

*Effects:* Adds a function object `g`, that performs
`DECAY_COPY(forward<Func>(f))(DECAY_COPY(forward<Args>(args))...)`, to the
`thread_pool`, where `DECAY_COPY` is evaluated in the thread that called
`post`.

```
template<class Func, class Args...>
  void defer(Func&& f, Args&&... args) const;
template<class ProtoAllocator, class Func, class Args...>
  void defer(allocator_arg_t, const ProtoAllocator& a,
    Func&& f, Args&&... args) const;
```

*Effects:* Adds a function object `g`, that performs
`DECAY_COPY(forward<Func>(f))(DECAY_COPY(forward<Args>(args))...)`, to the
`thread_pool`, where `DECAY_COPY` is evaluated in the thread that called
`post`.

```
template<class Function>
  void sync_execute(Function&& f) const;
```

*Effects:* If `running_in_this_thread()` is `true`, calls
`DECAY_COPY(forward<Func>(f))()`. Otherwise, adds `f` to the `thread_pool` and
blocks the caller pending completion of `f`.

*Returns:* The return value of `f`.

*Throws:* Any uncaught exception thrown by `f`.

```
template<class Function>
  future<result_of_t<decay_t<Function>()>>
    async_execute(Function&& f) const;
```

*Effects:* Creates an asynchronous provider with an associated shared state
(C++Std [futures.state]). Adds `f` to the `thread_pool`. On successful
completion of `f`, the return value of `f` is atomically stored in the shared
state and the shared state is made ready. If `f` exits via an exception, the
exception is atomically stored in the shared state and the shared state is made
ready.

*Returns:* An object of type `future<result_of_t<decay_t<Function>>()>` that
refers to the shared state created by `async_execute`.
