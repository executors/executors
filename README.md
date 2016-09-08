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


# Pattern for a self contained executor concept


For extensibility *executor* cannot be a specific, standardized class.
Instead an *executor* is a class that conforms to a standardized concepts,
semantics, and interface patterns.
This is similar to interators where a specific class is not standardized
but instead semantic / behavioral properties are standardized with
standardized mechanisms to observe those semantics.


An instruction stream is supplied to an execution function as
an object of a type that satisfies ``std::is_callable``.
The callable interface of this object has leading arguments
defined by the execution agent that will run the object
and trailing arguments that are passed through the execution function.
Either of these argument lists may be empty.


Different interface patterns can be used to express executor type traits.
One pattern is through external metafunctions, following the pattern
of metafunctions use to observe the type traits of built types;
e.g., `std::is_signed<T>`.
Another pattern assumes an executor is a class interface and
embeds traits as member type aliases, `constexpr` accessible values,
or similar mechanisms; e.g., any standard container.


## Illustrative pattern for some self-contained executor

    struct some_executor {
      using architecture = /* instruction stream target architecture */ ;
      /* ... other type properties ... */

      constexpr bool is_one_way_executor = /* property */ ;
      /* ... other value properties ... */

      constexpr architecture resource() const ; /* execution resource of this executor */

      using agent_policy = /* policy for creating execution agent(s) */ ;
      using agent_id     = /* identifier for created execution agent(s) */

      template< typename F , typename ... Args >
      using return_t = /* given closure F and arguments Args... the return type of the execute function */
      
      // requires: ! is_same_v<agent_policy,void>
      template< typename F , typename ... Args >
      return_t< T , Args... >
      execute( agent_spec , F && , Args && ... );

      // requires: is_same_v<agent_policy,void>
      template< typename F , typename ... Args >
      return_t< T , Args... >
      execute( F && , Args && ... );

      // if ! is_same_v<agent_id,void>
      //   requires is_callable_v< decay_t<F>( decay_t<Args>... ) >
      // else
      //   requires is_callable_v< decay_t<F>( agent_id , decay_t<Args>... ) >
    };


###  Example of a self contained executor underpinning `std::async`

    class async_host_executor {
    public:
      /* ... type properties ... */
      /* ... value properties ... */

      using architecture  = host_process ;
      using agent_policy  = launch ;
      using agent_id      = void ;
   
      constexpr architecture resource() const ;
   
      template <typename F, typename...Args>
      using return_t = future< result_of_t< decay_t<F>(decay_t<Args>...) > > ;
   
      template <typename F, typename...Args>
      return_t< F , Args... >
      execute( exec_policy , F&& , Args&& ... );
    };


# Minimal executor category

## Executor type traits

### Checking that a type is a `OneWayExecutor`

    template<class T> struct is_one_way_executor : see-below;

    template<class T> constexpr bool is_one_way_executor_v = is_one_way_executor<T>::value;

`is_one_way_executor<T>` publicly inherits from `std::true_type` if `T` satisfies the `OneWayExecutor` requirements (see Table \ref{one_way_executor_requirements}); otherwise, it publicly inherits from `std::false_type`.

XXX Is this implementable? For example, there's no way to check that `T::spawn_execute(f)` is valid for all `f` if all we have is `T`. However, we could do something like check with `function<void()>`, and
    insist that if it works for `function<void()>`, then it must work for all nullary functions.

### Checking that a type is a `TwoWayExecutor`

    template<class T> struct is_two_way_executor : see-below;

    template<class T> constexpr bool is_two_way_executor_v = is_two_way_executor<T>::value;

`is_two_way_executor<T>` publicly inherits from `std::true_type` if `T` satisfies the `TwoWayExecutor` requirements (see Table \ref{two_way_executor_requirements}); otherwise, it publicly inherits from `std::false_type`.

XXX Is this implementable? For example, there's no way to check that `T::async_execute(f)` is valid for all `f` if all we have is `T`. However, we could do something like check with `function<void()>`, and
    insist that if it works for `function<void()>`, then it must work for all nullary functions.
        

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

## `OneWayExecutor`

1. The `OneWayExecutor` requirements form the basis of the one-way executor concept taxonomy;
   every one-way executor satisfies the `OneWayExecutor` requirements. This set of requirements
   specifies operations for creating execution agents that need not synchronize with the thread
   which created them.

2. In Table \ref{one_way_executor_requirements}, `f`, denotes a `MoveConstructible` function object with zero arguments
   and `x` denotes an object of type `X`.

3. A type `X` satisfies the `OneWayExecutor` requirements if:
  * `X` satisfies the `CopyConstructible` requirements (17.6.3.1).
  * For any `f` and `x`, the expressions in Table \ref{one_way_executor_requirements} are valid and have the indicated semantics.

Table: (One-Way Executor requirements) \label{one_way_executor_requirements}

| Expression                                                                         | Return Type                                                   | Operational semantics                                                    | Assertion/note/pre-/post-condition                                 |
|------------------------------------------------------------------------------------|---------------------------------------------------------------|--------------------------------------------------------------------------|--------------------------------------------------------------------|
| `x.spawn_-` `execute(std::move(f))`                                                | `void`                                                        |  Creates an execution agent which invokes `f()`                          |                                                                    |
|                                                                                    |                                                               |                                                                          |                                                                    |
|                                                                                    |                                                               |                                                                          |                                                                    |


## `TwoWayExecutor`

1. The `TwoWayExecutor` requirements form the basis of the two-way executor concept taxonomy;
   every two-way executor satisfies the `TwoWayExecutor` requirements. This set of requirements
   specifies operations for creating execution agents that synchronize with the thread
   which created them.

2. In Table \ref{one_way_executor_requirements}, `f`, denotes a `MoveConstructible` function object with zero arguments whose result type is `R`,
   and `x` denotes an object of type `X`.

3. A type `X` satisfies the `OneWayExecutor` requirements if:
  * `X` satisfies the `CopyConstructible` requirements (17.6.3.1).
  * For any `f` and `x`, the expressions in Table \ref{one_way_executor_requirements} are valid and have the indicated semantics.

Table: (Two-Way Executor requirements) \label{two_way_executor_requirements}

| Expression                                                                         | Return Type                                                   | Operational semantics                                                    | Assertion/note/pre-/post-condition                                 |
|------------------------------------------------------------------------------------|---------------------------------------------------------------|--------------------------------------------------------------------------|--------------------------------------------------------------------|
| `x.async_-` `execute(std::move(f))`                                                | `executor_-` `future_t<X,R>`                                  |  Creates an execution agent which invokes `f()`                          |                                                                    |
|                                                                                    |                                                               |  Returns the result of `f()` via the resulting future object             |                                                                    |

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

## `BulkExecutor`

1. The `BulkExecutor` requirements form the basis of the bulk executor concept.
   This set of requirements specifies operations for creating groups of execution agents in bulk from a single operation.

2. In Table \ref{bulk_executor_requirements},
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
   \ref{bulk_executor_requirements} are valid and have the indicated semantics.

Table: (Bulk executor requirements) \label{bulk_executor_requirements}

| Expression                                                        | Return Type                                                       |  Operational semantics                                                                                     | Assertion/note/pre-/post-condition                                                                                                                         |
|-------------------------------------------------------------------|-------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `x.bulk_-` `execute(f, n, rf, sf)`                                | `R`                                                               |  Creates a group of execution agents of shape `n` which invoke `f(i, r, s)`                                | Note: blocks the forward progress of the caller until all invocations of `f` are finished.                                                                 |
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

# (Networking TS) executor category

XXX TODO

# Executor Customization Points

## In general

1. The functions described in this clause are *executor customization points*.
   Executor customization points provide a uniform interface to all executor types.

2. An executor customization point does not participate in overload resolution
   if its `exec` parameter is neither a `OneWayExecutor` nor a `TwoWayExecutor`. Executor customization points
   follow the design suggested by [N4381](wg21.link/N4381).

   XXX the reason p2 is included is to define some general purpose wording for executor customization points in order to avoid repetition below.
       but, there's still some repetition

## Function template `execution::spawn_execute()`

1.  ```
    template<class Executor, class Function>
    void spawn_execute(Executor& exec, Function&& f);
    ```

2. *Effects:* calls `exec.spawn_execute(std::forward<Function>(f))` if that call is well-formed; otherwise, calls
   `DECAY_COPY(std::forward<Function>(f))()` in a new execution agent with the call to `DECAY_COPY()` being evaluated
   in the thread that called `spawn_execute`. Any return value is discarded.

## Function template `execution::execute()`

1.  ```
    template<class Executor, class Function>
    result_of_t<decay_t<Function>()>
    execute(Executor& exec, Function&& f);
    ```

2. *Effects:* calls `exec.execute(std::forward<Function>(f))` if that call is well-formed; otherwise, calls
   `DECAY_COPY(std::forward<Function>(f))()` in a new execution agent with the call to `DECAY_COPY()` being
   evaluated in the thread that called `execute`.

3. *Returns:* The return value of `f`.

4. *Synchronization:* The invocation of `execute` synchronizes with (1.10) the invocation of `f`.

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

3. *Returns:* `executor_future_t<Executor,result_of_t<decay_t<Function>()>` when `predecessor` is a `void` future. Otherwise,
   `executor_future_t<Executor,result_of_t<decay_t<Function>(T&)>>` where `T` is the result type of the `predecessor` future.

4. *Synchronization:*
    * the invocation of `then_execute` synchronizes with (1.10) the invocation of `f`.
    * the completion of the invocation of `f` is sequenced before (1.10) the shared state is made ready.

5. *Postconditions:* If the `predecessor` future is not a shared future, then `predecessor.valid() == false`.


## Function template `execution::bulk_execute()`

1.  ```
    template<class Executor, class Function1, class Function2, class Function3>
    result_of_t<Function2(executor_shape_t<Executor>)>
    bulk_execute(Executor& exec, Function1 f, executor_shape_t<Executor> shape,
                 Function2 result_factory, Function3 shared_factory);
    ```

2. *Effects:* calls `exec.bulk_execute(f, shape, result_factory, shared_factory)` if that call is well-formed;
   otherwise:
   
   * Calls `result_factory(shape)` and `shared_factory(shape)` in an unspecified execution agent. The results
     of these invocations are stored to shared state.

   * Creates a new group of execution agents of shape `shape`. Each execution agent calls `f(idx, result, shared)`, where
     `idx` is the index of the execution agent, and `result` and `shared` are references to the respective shared state.
     Any return value of `f` is discarded.

3. *Returns:* An object of type `result_of_t<Function2(executor_shape_t<Executor>)>` that refers to the result shared state created by
   this call to `bulk_execute`.

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

3. *Returns:* An object of type

    `executor_future_t<Executor,result_of_t<Function2(executor_shape_t<Executor>)>`
    
    that refers to the shared result state created by this call to `bulk_then_execute`.

4. *Synchronization:*
    * the invocation of `bulk_then_execute` synchronizes with (1.10) the invocations of `f`.
    * the completion of the functions `result_factory` and `shared_factory` happen before the creation of the group of execution agents.
    * the completion of the invocations of `f` are sequenced before (1.10) the result shared state is made ready.

5. *Postconditions:* If the `predecessor` future is not a shared future, then `predecessor.valid() == false`.

## Networking TS-specific customization points

XXX TODO

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

2. The type of an execution policy's associated executor shall satisfy the requirements of `BulkExecutor`.

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
   `BulkExecutor`, the returned execution policy's associated executor is equivalent to `exec`. Otherwise,
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

    `return execution::execute(exec, [&]{ return INVOKE(f, args...); });`

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
    void spawn_execute(Function&& f);

    // XXX future proposals can introduce member functions for each
    //     Executor customization point

  private:
    std::any contained_executor_; // exposition only
};
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

#### Member function `spawn_execute`

1.  ```
    template<class Function>
    void spawn_execute(Function&& f);
    ```

2. Let `exec` be this `one_way_executor`'s contained executor.

3. *Effects:* As if by `execution::spawn_execute(exec, std::forward<Function>(f))`.

### Non-member functions

1.  ```
    void swap(one_way_executor& x, one_way_executor& y) noexcept;
    ```

2. *Effects:* As if by `x.swap(y)`.


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
```

1. An object of class `two_way_executor` stores an instance of any `OneWayExecutor` type. The stored instance is called the *contained executor*.

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
    // executor definitions
    // XXX should probably define one of each executor type in the
    // proposal.
    class basic_executor
    {
      public:
        template <typename T>
        using future = std::future<T>;

        template<class Executor, class Function>
        void spawn_execute(Function&& f);

        template<class Executor, class Function>
        future<result_of_t<decay_t<Function>()>>
        async_execute(Function&& f);
    };
    
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

    // placeholder for a general approach to getting executors from 
    // standard contexts.
    basic_executor get_executor() noexcept;
};
```

### Executor properties

1.  ```
    class basic_executor;
    ```

2. An executor type fulfilling both the `OneWayExecutor` and `TwoWayExecutor` requirements.

### Construction and destruction

1.  ```
    thread_pool(std::size_t num_threads);
    ```

2. *Effects:* Constructs a `thread_pool` object with an implementation defined
    number of threads of execution. Additionally starts the worker threads.

3.  ```
    thread_pool(std::size_t num_threads);
    ```

4. *Effects:* Constructs a `thread_pool` object with the provided number of threads of execution. Additionally starts the worker threads.

5.  ```
    ~thread_pool();
    ```

6. *Effects:* Stops the pool from accepting new tasks and blocks the calling thread on the completion of execution of all work in the thread pool. 

### Worker Management

1.  ```
    void attach();
    ```

2. *Effects:* adds the calling thread to the pool of workers and block caller until the pool is destroyed.


### Executor Creation

1.  ```
    basic_executor get_executor() noexcept;
    ```

2. *Returns:* an executor object satisfying the `OneWayExecutor` and `TwoWayExecutor` requirements.

