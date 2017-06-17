% A Unified Executors Proposal for C++ | D0443R2

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

Document Number:    D0443R2

Date:               2017-XX-XX

Audience:           SG1 - Concurrency and Parallelism

Reply-to:           jhoberock@nvidia.com

------------------------------------------------------

# Abstract

This paper proposes a programming model for executors, which are modular
components for creating execution. The design of this proposal is described in
paper DXXXX.

## Changelog

### Changes since R0

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

### Changes since R1

* Separated wording from explanatory prose

# Proposed Wording

### Header `<execution>` synopsis

```
namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

  // Directionality properties:

  constexpr struct oneway_t {} oneway;
  constexpr struct twoway_t {} twoway;

  // Cardinality properties:

  constexpr struct single_t {} single;
  constexpr struct bulk_t {} bulk;

  // Blocking properties:

  constexpr struct never_blocking_t {} never_blocking;
  constexpr struct possibly_blocking_t {} possibly_blocking;
  constexpr struct always_blocking_t {} always_blocking;

  // Properties to indicate if submitted tasks represent continuations:

  constexpr struct is_continuation_t {} is_continuation;
  constexpr struct is_not_continuation_t {} is_not_continuation;

  // Properties to indicate likely task submission in the future:

  constexpr struct is_work_t {} is_work;
  constexpr struct is_not_work_t {} is_not_work;

  // Properties for bulk execution forward progress guarantees:

  constexpr struct bulk_sequenced_execution_t {} bulk_sequenced_execution;
  constexpr struct bulk_parallel_execution_t {} bulk_parallel_execution;
  constexpr struct bulk_unsequenced_execution_t {} bulk_unsequenced_execution;

  // Properties for mapping of execution on to threads:

  constexpr struct thread_execution_mapping_t {} thread_execution_mapping;
  constexpr struct new_thread_execution_mapping_t {} new_thread_execution_mapping;

  // Executor type traits:

  template<class Executor> struct is_executor;
  template<class Executor> struct is_oneway_executor;
  template<class Executor> struct is_twoway_executor;
  template<class Executor> struct is_bulk_oneway_executor;
  template<class Executor> struct is_bulk_twoway_executor;

  template<class Executor> constexpr bool is_executor_v = is_executor<Executor>::value;
  template<class Executor> constexpr bool is_oneway_executor_v = is_oneway_executor<Executor>::value;
  template<class Executor> constexpr bool is_twoway_executor_v = is_twoway_executor<Executor>::value;
  template<class Executor> constexpr bool is_bulk_oneway_executor_v = is_bulk_oneway_executor<Executor>::value;
  template<class Executor> constexpr bool is_bulk_twoway_executor_v = is_bulk_twoway_executor<Executor>::value;

  template<class Executor> struct executor_context;
  template<class Executor, class T> struct executor_future;
  template<class Executor> struct executor_shape;
  template<class Executor> struct executor_index;

  template<class Executor> using executor_context_t = typename executor_context<Executor>::type;
  template<class Executor, class T> using executor_future_t = typename executor_future<Executor, T>::type;
  template<class Executor> using executor_shape_t = typename executor_shape<Executor>::type;
  template<class Executor> using executor_index_t = typename executor_index<Executor>::type;

  // Member detection type traits for properties:

  template<class Executor, class... Args> struct has_require_member;
  template<class Executor, class... Args> struct has_prefer_member;

  template<class Executor, class... Args> constexpr bool has_require_member_v = has_require_member<Executor, Args...>::value;
  template<class Executor, class... Args> constexpr bool has_prefer_member_v = has_prefer_member<Executor, Args...>::value;

  // Member return type traits for properties:

  template<class Executor, class... Args> struct require_member_result;
  template<class Executor, class... Args> struct prefer_member_result;

  template<class Executor, class... Args> using require_member_result_t = typename require_member_result<Executor, Args...>::type;
  template<class Executor, class... Args> using prefer_member_result_t = typename prefer_member_result<Executor, Args...>::type;

  // Customization points:

  namespace {
    constexpr unspecified require = unspecified;
    constexpr unspecified prefer = unspecified;
  }

  // Customization point type traits:

  template<class Executor, class... Args> struct can_require;
  template<class Executor, class... Args> struct can_prefer;

  template<class Executor, class... Args> constexpr bool can_require_v = can_require<Executor, Args...>::value;
  template<class Executor, class... Args> constexpr bool can_prefer_v = can_prefer<Executor, Args...>::value;

  // Polymorphic executor wrappers:

  class bad_executor;
  class executor;

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std
```

## Requirements

### Customization point objects

*(The following text has been adapted from the draft Ranges Technical Specification.)*

A *customization point object* is a function object (C++ Std, [function.objects]) with a literal class type that interacts with user-defined types while enforcing semantic requirements on that interaction.

The type of a customization point object shall satisfy the requirements of `CopyConstructible` (C++Std [copyconstructible]) and `Destructible` (C++Std [destructible]).

All instances of a specific customization point object type shall be equal.

Let `t` be a (possibly const) customization point object of type `T`, and `args...` be a parameter pack expansion of some parameter pack `Args...`. The customization point object `t` shall be callable as `t(args...)` when the types of `Args...` meet the requirements specified in that customization point object's definition. Otherwise, `T` shall not have a function call operator that participates in overload resolution.

Each customization point object type constrains its return type to satisfy some particular type requirements.

The library defines several named customization point objects. In every translation unit where such a name is defined, it shall refer to the same instance of the customization point object.

[*Note:* Many of the customization points objects in the library evaluate function call expressions with an unqualified name which results in a call to a user-defined function found by argument dependent name lookup (C++Std [basic.lookup.argdep]). To preclude such an expression resulting in a call to unconstrained functions with the same name in namespace `std`, customization point objects specify that lookup for these expressions is performed in a context that includes deleted overloads matching the signatures of overloads defined in namespace `std`. When the deleted overloads are viable, user-defined overloads must be more specialized (C++Std [temp.func.order]) to be used by a customization point object. *--end note*]

### `Future` requirements

A type `F` meets the `Future` requirements for some value type `T` if `F` is `std::experimental::future<T>` (defined in the C++ Concurrency TS, ISO/IEC TS 19571:2016).  [*Note:* This concept is included as a placeholder to be elaborated, with the expectation that the elaborated requirements for `Future` will expand the applicability of some executor customization points. *--end note*]

### `ProtoAllocator` requirements

A type `A` meets the `ProtoAllocator` requirements if `A` is `CopyConstructible` (C++Std [copyconstructible]), `Destructible` (C++Std [destructible]), and `allocator_traits<A>::rebind_alloc<U>` meets the allocator requirements (C++Std [allocator.requirements]), where `U` is an object type. [*Note:* For example, `std::allocator<void>` meets the proto-allocator requirements but not the allocator requirements. *--end note*] No comparison operator, copy operation, move operation, or swap operation on these types shall exit via an exception.

### `ExecutionContext` requirements

A type meets the `ExecutionContext` requirements if it satisfies the `EqualityComparable` requirements (C++Std [equalitycomparable]). No comparison operator on these types shall exit via an exception.

### `BaseExecutor` requirements

A type `X` meets the `BaseExecutor` requirements if it satisfies the requirements of `CopyConstructible` (C++Std [copyconstructible]), `Destructible` (C++Std [destructible]), and `EqualityComparable` (C++Std [equalitycomparable]), as well as the additional requirements listed below.

No comparison operator, copy operation, move operation, swap operation, or member function `context` on these types shall exit via an exception.

The executor copy constructor, comparison operators, `context` member function, associated execution functions, and other member functions defined in refinements (TODO: what should this word be?) of the `BaseExecutor` requirements shall not introduce data races as a result of concurrent calls to those functions from different threads.

The destructor shall not block pending completion of the submitted function objects. [*Note:* The ability to wait for completion of submitted function objects may be provided by the associated execution context. *--end note*]

In the Table \ref{base_executor_requirements} below, `x1` and `x2` denote (possibly const) values of type `X`, `mx1` denotes an xvalue of type `X`, and `u` denotes an identifier.

Table: (Base executor requirements) \label{base_executor_requirements}

| Expression   | Type       | Assertion/note/pre-/post-condition |
|--------------|------------|------------------------------------|
| `X u(x1);` | | Shall not exit via an exception. <br/><br/>*Post:* `u == x1` and `u.context() == x1.context()`. |
| `X u(mx1);` | | Shall not exit via an exception. <br/><br/>*Post:* `u` equals the prior value of `mx1` and `u.context()` equals the prior value of `mx1.context()`. |
| `x1 == x2` | `bool` | Returns `true` only if `x1` and `x2` can be interchanged with identical effects in any of the expressions defined in these type requirements (TODO and the other executor requirements defined in this Technical Specification). [*Note:* Returning `false` does not necessarily imply that the effects are not identical. *--end note*] `operator==` shall be reflexive, symmetric, and transitive, and shall not exit via an exception. |
| `x1 != x2` | `bool` | Same as `!(x1 == x2)`. |
| `x1.context()` | `E&` or `const E&` where `E` is a type that satisfies the `ExecutionContext` requirements. | Shall not exit via an exception. The comparison operators and member functions defined in these requirements (TODO and the other executor requirements defined in this Technical Specification) shall not alter the reference returned by this function. |

[*Commentary: The equality operator is specified primarily as an aid to support postconditons on executor copy construction and move construction. The key word in supporting these postconditions is "interchanged". That is, if a copy is substituted for the original executor it shall produce identical effects, provided the expression, calling context, and program state are otherwise identical. Calls to the copied executor from a different context or program state are not required to produce identical effects, and this is not considered an "interchanged" use of an executor. In particular, even consecutive calls to the same executor need not produce identical effects since the program state has already altered.*]

### `OneWayExecutor` requirements

The `OneWayExecutor` requirements specify requirements for executors which create execution agents without a channel for awaiting the completion of a submitted function object and obtaining its result. [*Note:* That is, the executor provides fire-and-forget semantics. *--end note*]

A type `X` satisfies the `OneWayExecutor` requirements if it satisfies the `BaseExecutor` requirements, as well as the requirements in the table below.

In the Table below, `x` denotes a (possibly const) executor object of type `X` and `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))()` and where `decay_t<F>` satisfies the `MoveConstructible` requirements.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.execute(f)` | `void` | Creates an execution agent which invokes `DECAY_COPY( std::forward<F>(f))()` at most once, with the call to `DECAY_COPY` being evaluated in the thread that called `execute`. <br/> <br/> May block forward progress of the caller until `DECAY_COPY( std::forward<F>(f))()` finishes execution. <br/> <br/> The invocation of `execute` synchronizes with (C++Std [intro.multithread]) the invocation of `f`. <br/> <br/> `execute` shall not propagate any exception thrown by `DECAY_COPY( std::forward<F>(f))()` or any other function submitted to the executor. [*Note:* The treatment of exceptions thrown by one-way submitted functions is specific to the concrete executor type. *--end note.*] |

### `TwoWayExecutor` requirements

The `TwoWayExecutor` requirements specify requirements for executors which creating execution agents with a channel for awaiting the completion of a submitted function object and obtaining its result.

A type `X` satisfies the `TwoWayExecutor` requirements if it satisfies the `BaseExecutor` requirements, as well as the requirements in the table below.

In the Table below, `x` denotes a (possibly const) executor object of type `X`, `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))()` and where `decay_t<F>` satisfies the `MoveConstructible` requirements, and `R` denotes the type of the expression `DECAY_COPY(std::forward<F>(f))()`.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.twoway_executor(f)` | A type that satisfies the `Future` requirements for the value type `R`. | Creates an execution agent which invokes `DECAY_COPY( std::forward<F>(f))()` at most once, with the call to `DECAY_COPY` being evaluated in the thread that called `execute`. <br/> <br/> May block forward progress of the caller until `DECAY_COPY( std::forward<F>(f))()` finishes execution. <br/> <br/> The invocation of `execute` synchronizes with (C++Std [intro.multithread]) the invocation of `f`. <br/> <br/> Stores the result of `DECAY_COPY( std::forward<F>(f))()`, or any exception thrown by `DECAY_COPY( std::forward<F>(f))()`, in the associated shared state of the resulting `Future`. |

### `BulkOneWayExecutor` requirements

The `BulkOneWayExecutor` requirements specify requirements for executors which create groups of execution agents in bulk from a single execution function, without a channel for awaiting the completion of the submitted function object invocations and obtaining their result. [*Note:* That is, the executor provides fire-and-forget semantics. *--end note*]

A type `X` satisfies the `BulkOneWayExecutor` requirements if it satisfies the `BaseExecutor` requirements, as well as the requirements in the table below.

In the Table below,

  * `x` denotes a (possibly const) executor object of type `X`,
  * `n` denotes a shape object whose type is `executor_shape_t<X>`,
  * `sf` denotes a `CopyConstructible` function object with zero arguments whose result type is `S`,
  * `i` denotes a (possibly const) object whose type is `executor_index_t<X>`,
  * `s` denotes an object whose type is `S`,
  * `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))(i, s)` and where `decay_t<F>` satisfies the `MoveConstructible` requirements.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.bulk_execute(f, n, sf)` | `void` | Invokes `sf()` on an unspecified execution agent to produce the value `s`. Creates a group of execution agents of shape `n` which invokes `DECAY_COPY( std::forward<F>(f))(i, s)` at most once for each value of `i` in the range `[0,n)`, with the call to `DECAY_COPY` being evaluated in the thread that called `bulk_execute`. <br/> <br/> May block forward progress of the caller until one or more calls to `DECAY_COPY( std::forward<F>(f))(i, s)` finish execution. <br/> <br/> The invocation of `bulk_execute` synchronizes with (C++Std [intro.multithread]) the invocations of `f`. <br/> <br/> `bulk_execute` shall not propagate any exception thrown by `DECAY_COPY( std::forward<F>(f))(i, s)` or any other function submitted to the executor. [*Note:* The treatment of exceptions thrown by bulk one-way submitted functions is specific to the concrete executor type. *--end note.*] |

### `BulkTwoWayExecutor` requirements

The `BulkTwoWayExecutor` requirements specify requirements for executors which create groups of execution agents in bulk from a single execution function with a channel for awaiting the completion of a submitted function object invoked by those execution agents and obtaining its result.

A type `X` satisfies the `BulkTwoWayExecutor` requirements if it satisfies the `BaseExecutor` requirements, as well as the requirements in the table below.

In the Table below,

  * `x` denotes a (possibly const) executor object of type `X`,
  * `n` denotes a shape object whose type is `executor_shape_t<X>`,
  * `rf` denotes a `CopyConstructible` function object with zero arguments whose result type is `R`,
  * `sf` denotes a `CopyConstructible` function object with zero arguments whose result type is `S`,
  * `i` denotes a (possibly const) object whose type is `executor_index_t<X>`,
  * `s` denotes an object whose type is `S`,
  * if `R` is non-void,
    * `r` denotes an object whose type is `R`,
    * `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))(i, r, s)` and where `decay_t<F>` satisfies the `MoveConstructible` requirements.
  * if `R` is void,
    * `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))(i, s)` and where `decay_t<F>` satisfies the `MoveConstructible` requirements.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.bulk_twoway_execute(f, n, rf, sf)` | A type that satisfies the `Future` requirements for the value type `R`. | If `R` is non-void, invokes `rf()` on an unspecified execution agent to produce the value `r`. Invokes `sf()` on an unspecified execution agent to produce the value `s`. Creates a group of execution agents of shape `n` which invokes `DECAY_COPY( std::forward<F>(f))(i, r, s)` if `R` is non-void, and otherwise invokes `DECAY_COPY( std::forward<F>(f))(i, s)`, at most once for each value of `i` in the range `[0,n)`, with the call to `DECAY_COPY` being evaluated in the thread that called `bulk_twoway_execute`. <br/> <br/> May block forward progress of the caller until one or more invocations of `f` finish execution. <br/> <br/> The invocation of `bulk_twoway_execute` synchronizes with (C++Std [intro.multithread]) the invocations of `f`. <br/> <br/> Once all invocations of `f` finish execution, stores `r`, or any exception thrown by an invocation of `f`, in the associated shared state of the resulting `Future`. |

## Executor properties

### In general

### Directionality properties

    constexpr struct oneway_t {} oneway;
    constexpr struct twoway_t {} twoway;

### Cardinality properties

    constexpr struct single_t {} single;
    constexpr struct bulk_t {} bulk;

### Blocking properties

    constexpr struct never_blocking_t {} never_blocking;
    constexpr struct possibly_blocking_t {} possibly_blocking;
    constexpr struct always_blocking_t {} always_blocking;

#### Properties to indicate if submitted tasks represent continuations

    constexpr struct is_continuation_t {} is_continuation;
    constexpr struct is_not_continuation_t {} is_not_continuation;

### Properties to indicate likely task submission in the future

    constexpr struct is_work_t {} is_work;
    constexpr struct is_not_work_t {} is_not_work;

### Properties for bulk execution forward progress guarantees

    constexpr struct bulk_sequenced_execution_t {} bulk_sequenced_execution;
    constexpr struct bulk_parallel_execution_t {} bulk_parallel_execution;
    constexpr struct bulk_unsequenced_execution_t {} bulk_unsequenced_execution;

### Properties for mapping of execution on to threads

    constexpr struct thread_execution_mapping_t {} thread_execution_mapping;
    constexpr struct new_thread_execution_mapping_t {} new_thread_execution_mapping;

### Properties for customizing memory allocation

TODO using std::allocator_arg_t

## Executor type traits

### Determining that a type satisfies executor type requirements

    template<class T> struct is_executor;
    template<class T> struct is_oneway_executor;
    template<class T> struct is_twoway_executor;
    template<class T> struct is_bulk_oneway_executor;
    template<class T> struct is_bulk_twoway_executor;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class T>` <br/>`struct is_executor` | `T` meets the syntactic requirements for `BaseExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_oneway_executor` | `T` meets the syntactic requirements for `OneWayExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_twoway_executor` | `T` meets the syntactic requirements for `TwoWayExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_bulk_oneway_executor` | `T` meets the syntactic requirements for `BulkOneWayExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_bulk_twoway_executor` | `T` meets the syntactic requirements for `BulkTwoWayExecutor`. | `T` is a complete type. |

### Associated execution context type

    template<class Executor>
    struct executor_context
    {
      using type = std::decay_t<decltype(declval<const Executor&>().context())>;
    };

### Associated future type

    template<class Executor, class T>
    struct executor_future
    {
      using type = see below;
    };
    
The type of `executor_future<Executor, T>::type` is determined as follows:

* if `is_twoway_executor<Executor>` is true, `decltype(declval<const Executor&>().twoway_execute( declval<T(*)()>())`;

* otherwise, if `is_oneway_executor<Executor>` is true, `std::experimental::future<T>`;

* otherwise, the program is ill formed.

[*Note:* The effect of this specification is that all execute functions of an executor that satisfies the `TwoWayExecutor`, `NeverBlockingTwoWayExecutor`, or `BulkTwoWayExecutor` requirements must utilize the same future type, and that this future type is determined by `async_execute`. Programs may specialize this trait for user-defined `Executor` types. *--end note*]

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

### Member detection type traits for properties

    template<class Executor, class... Args> struct has_require_member;
    template<class Executor, class... Args> struct has_prefer_member;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class T>` <br/>`struct has_require_member` | The expression `declval<const Executor>().require( declval<Args>()...)` is well formed. | `T` is a complete type. |
| `template<class T>` <br/>`struct has_prefer_member` | The expression `declval<const Executor>().prefer( declval<Args>()...)` is well formed. | `T` is a complete type. |

### Member return type traits for properties

    template<class Executor, class... Args> struct require_member_result;
    template<class Executor, class... Args> struct prefer_member_result;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a TransformationTrait (C++Std [meta.rqmts]).

| Template                   | Condition           | Comments  |
|----------------------------|---------------------|-----------|
| `template<class T>` <br/>`struct require_member_result` | The expression `declval<const Executor>().require( declval<Args>()...)` is well formed. | The member typedef `type` shall name the type of the expression `declval<const Executor>().require( declval<Args>()...)`. |
| `template<class T>` <br/>`struct prefer_member_result` | The expression `declval<const Executor>().prefer( declval<Args>()...)` is well formed. | The member typedef `type` shall name the type of the expression `declval<const Executor>().prefer( declval<Args>()...)`. |

## Executor customization points

*Executor customization points* are functions which adapt an executor's properties. Executor customization points enable uniform use of executors in generic contexts.

When an executor customization point named *NAME* invokes a free execution function of the same name, overload resolution is performed in a context that includes the declaration `void` *NAME*`(auto&... args) = delete;`, where `sizeof...(args)` is the arity of the free execution function. This context also does not include a declaration of the executor customization point.

[*Note:* This provision allows executor customization points to call the executor's free, non-member execution function of the same name without recursion. *--end note*]

Whenever `std::experimental::concurrency_v2::execution::`*NAME*`(`*ARGS*`)` is a valid expression, that expression satisfies the syntactic requirements for the free execution function named *NAME* with arity `sizeof...(`*ARGS*`)` with that free execution function's semantics.

### `require`

    namespace {
      constexpr unspecified require = unspecified;
    }

The name `require` denotes a customization point. The effect of the expression `std::experimental::concurrency_v2::execution::require(E, P, A...)` for some expressions `E` and `F`, and where `A...` represents 0 or more expressions, is equivalent to:

* `(E).require(F, P, A...)` if `has_require_member_v<decay_t<decltype(E)>, P, A...>` is true.

* Otherwise, `std::experimental::concurrency_v2::execution::require(E, P, A...)` is ill-formed.

### `prefer`

    namespace {
      constexpr unspecified prefer = unspecified;
    }

The name `prefer` denotes a customization point. The effect of the expression `std::experimental::concurrency_v2::execution::prefer(E, P, A...)` for some expressions `E` and `F`, and where `A...` represents 0 or more expressions, is equivalent to:

* `(E).prefer(F, P, A...)` if `has_prefer_member_v<decay_t<decltype(E)>, P, A...>` is true.

* Otherwise, `std::experimental::concurrency_v2::execution::prefer(E, P, A...)` is ill-formed.

### Customization point type traits

    template<class Executor, class... Args> struct can_require;
    template<class Executor, class... Args> struct can_prefer;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class T>` <br/>`struct can_require` | The expression `std::experimental::concurrency_v2::execution::require( declval<const Executor>(), declval<Args>()...)` is well formed. | `T` is a complete type. |
| `template<class T>` <br/>`struct can_prefer` | The expression `std::experimental::concurrency_v2::execution::prefer( declval<const Executor>(), declval<Args>()...)` is well formed. | `T` is a complete type. |

## Polymorphic executor wrappers

### Class `bad_executor`

An exception of type `bad_executor` is thrown by `executor` member functions `execute`, `twoway_execute`, `bulk_execute`, and `bulk_twoway_execute` when the executor object has no target.

```
class bad_executor : public exception
{
public:
  // constructor:
  bad_executor() noexcept;
};
```

#### `bad_executor` constructors

```
bad_executor() noexcept;
```

*Effects:* Constructs a `bad_executor` object.

*Postconditions:* `what()` returns an implementation-defined NTBS.

### Class `executor`

The `executor` class provides a polymorphic wrapper for executor types.

```
class executor
{
public:
  class context_type;

  // construct / copy / destroy:

  executor() noexcept;
  executor(nullptr_t) noexcept;
  executor(const executor& e) noexcept;
  executor(executor&& e) noexcept;
  template<class Executor> executor(Executor e);

  executor& operator=(const executor& e) noexcept;
  executor& operator=(executor&& e) noexcept;
  executor& operator=(nullptr_t) noexcept;
  template<class Executor> executor& operator=(Executor e);

  ~executor();

  // executor modifiers:

  void swap(executor& other) noexcept;

  // executor operations:

  const context_type& context() const noexcept;

  executor require(oneway_t) const;
  executor require(twoway_t) const;
  executor require(single_t) const;
  executor require(bulk_t) const;
  executor require(thread_execution_mapping_t) const;

  executor require(never_blocking_t p) const;
  executor require(possibly_blocking_t p) const;
  executor require(always_blocking_t p) const;

  executor prefer(is_continuation_t p) const;
  executor prefer(is_not_continuation_t p) const;
  executor prefer(is_work_t p) const;
  executor prefer(is_not_work_t p) const;
  executor prefer(bulk_sequenced_execution_t p) const;
  executor prefer(bulk_parallel_execution_t p) const;
  executor prefer(bulk_unsequenced_execution_t p) const;
  executor prefer(new_thread_execution_mapping p) const;

  template <class... Args> executor prefer(Args&&...) const;

  template<class Function>
    void execute(Function&& f) const;

  template<class Function>
    std::experimental::future<result_of_t<decay_t<F>()>>
      twoway_execute(Function&& f) const

  template<class Function, class SharedFactory>
    void bulk_execute(Function&& f, size_t n, SharedFactory&& sf) const;

  template<class Function, class ResultFactory, class SharedFactory>
    std::experimental::future<result_of_t<decay_t<ResultFactory>()>>
      void bulk_twoway_execute(Function&& f, size_t n, ResultFactory&& rf, SharedFactory&& sf) const;

  // executor capacity:

  explicit operator bool() const noexcept;

  // executor target access:

  const type_info& target_type() const noexcept;
  template<class Executor> Executor* target() noexcept;
  template<class Executor> const Executor* target() const noexcept;
};

// executor comparisons:

bool operator==(const executor& a, const executor& b) noexcept;
bool operator==(const executor& e, nullptr_t) noexcept;
bool operator==(nullptr_t, const executor& e) noexcept;
bool operator!=(const executor& a, const executor& b) noexcept;
bool operator!=(const executor& e, nullptr_t) noexcept;
bool operator!=(nullptr_t, const executor& e) noexcept;

// executor specialized algorithms:

void swap(executor& a, executor& b) noexcept;
```

The `executor` class satisfies the `BaseExecutor`, `DefaultConstructible` (C++Std [defaultconstructible]), and `CopyAssignable` (C++Std [copyassignable]) requirements.

[*Note:* To meet the `noexcept` requirements for executor copy constructors and move constructors, implementations may share a target between two or more `executor` objects. *--end note*]

The *target* is the executor object that is held by the wrapper.

#### `executor` constructors

```
executor() noexcept;
```

*Postconditions:* `!*this`.

```
executor(nullptr_t) noexcept;
```

*Postconditions:* `!*this`.

```
executor(const executor& e) noexcept;
```

*Postconditions:* `!*this` if `!e`; otherwise, `*this` targets `e.target()` or a copy of `e.target()`.

```
executor(executor&& e) noexcept;
```

*Effects:* If `!e`, `*this` has no target; otherwise, moves `e.target()` or move-constructs the target of `e` into the target of `*this`, leaving `e` in a valid state with an unspecified value.

```
template<class Executor> executor(Executor e);
```

*Requires:*

  * `can_require_v<Executor, oneway>`
  * `can_require_v<Executor, twoway>`
  * `can_require_v<Executor, single>`
  * `can_require_v<Executor, bulk>`
  * `can_require_v<Executor, never_blocking>`
  * `can_require_v<Executor, possibly_blocking>`
  * `can_require_v<Executor, always_blocking>`
  * `can_prefer_v<Executor, is_continuation>`
  * `can_prefer_v<Executor, is_not_continuation>`
  * `can_prefer_v<Executor, is_work>`
  * `can_prefer_v<Executor, is_not_work>`
  * `can_prefer_v<Executor, bulk_sequenced_execution>`
  * `can_prefer_v<Executor, bulk_parallel_execution>`
  * `can_prefer_v<Executor, bulk_unsequenced_execution>`
  * `can_require_v<Executor, thread_execution_mapping>`
  * `can_prefer_v<Executor, new_thread_execution_mapping>`

*Effects:* `*this` targets a copy of `e` initialized with `std::move(e)`.

#### `executor` assignment

```
executor& operator=(const executor& e) noexcept;
```

*Effects:* `executor(e).swap(*this)`.

*Returns:* `*this`.

```
executor& operator=(executor&& e) noexcept;
```

*Effects:* Replaces the target of `*this` with the target of `e`, leaving `e` in a valid state with an unspecified value.

*Returns:* `*this`.

```
executor& operator=(nullptr_t) noexcept;
```

*Effects:* `executor(nullptr).swap(*this)`.

*Returns:* `*this`.

```
template<class Executor> executor& operator=(Executor e);
```

*Requires:* As for `template<class Executor> executor(Executor e)`.

*Effects:* `executor(std::move(e)).swap(*this)`.

*Returns:* `*this`.

#### `executor` destructor

```
~executor();
```

*Effects:* If `*this != nullptr`, releases shared ownership of, or destroys, the target of `*this`.

#### `executor` modifiers

```
void swap(executor& other) noexcept;
```

*Effects:* Interchanges the targets of `*this` and `other`.

#### `executor` operations

```
const context_type& context() const noexcept;
```

*Requires:* `*this != nullptr`.

*Returns:* A polymorphic wrapper for `e.context()`, where `e` is the target object of `*this`.

```
executor require(oneway_t) const;
executor require(twoway_t) const;
executor require(single_t) const;
executor require(bulk_t) const;
executor require(thread_execution_mapping_t) const;
```

*Returns:* `*this`.

```
executor require(never_blocking_t p) const;
executor require(possibly_blocking_t p) const;
executor require(always_blocking_t p) const;
```

*Returns:* A polymorphic wrapper whose target is `execution::require(e, p)`, where `e` is the target object of `*this`.

```
executor prefer(is_continuation_t) const;
executor prefer(is_not_continuation_t) const;
executor prefer(is_work_t) const;
executor prefer(is_not_work_t) const;
executor prefer(bulk_sequenced_execution_t) const;
executor prefer(bulk_parallel_execution_t) const;
executor prefer(bulk_unsequenced_execution_t) const;
executor prefer(new_thread_execution_mapping) const;
```

*Returns:* A polymorphic wrapper whose target is `execution::prefer(e, p)`, where `e` is the target object of `*this`.

```
template <class... Args> executor prefer(Args&&...) const;
```

*Returns:* `*this`.

```
template<class Function>
  void execute(Function&& f) const;
```

*Effects:* Performs `e2.execute(f2)`, where:
  * `e1` is the target object of `*this`;
  * `e2` is the result of `require(require(e1, single), oneway)`;
  * `f1` is the result of `DECAY_COPY(std::forward<Function>(f))`;
  * `f2` is a function object of unspecified type that, when called as `f2()`, performs `f1()`.

```
template<class Function>
  std::experimental::future<result_of_t<decay_t<Function>()>>
    twoway_execute(Function&& f) const
```

*Effects:* Performs `e2.twoway_execute(f2)`, where:
  * `e1` is the target object of `*this`;
  * `e2` is the result of `require(require(e1, single), twoway)`;
  * `f1` is the result of `DECAY_COPY(std::forward<Function>(f))`;
  * `f2` is a function object of unspecified type that, when called as `f2()`, performs `f1()`.

*Returns:* A future, whose shared state is made ready when the future returned by `e.twoway_execute(f2)` is made ready, containing the result of `f1()` or any exception thrown by `f1()`. [*Note:* `e2.twoway_execute(f2)` may return any future type that satisfies the Future requirements, and not necessarily `std::experimental::future`. One possible implementation approach is for the polymorphic wrapper to attach a continuation to the inner future via that object's `then()` member function. When invoked, this continuation stores the result in the outer future's associated shared and makes that shared state ready. *--end note*]


```
template<class Function, class SharedFactory>
  void bulk_execute(Function&& f, size_t n, SharedFactory&& sf) const;
```

*Effects:* Performs `e2.bulk_execute(f2, n, sf2)`, where:
  * `e1` is the target object of `*this`;
  * `e2` is the result of `require(require(e1, bulk), oneway)`;
  * `sf1` is the result of `DECAY_COPY(std::forward<SharedFactory>(sf))`;
  * `sf2` is a function object of unspecified type that, when called as `sf2()`, performs `sf1()`;
  * `s1` is the result of `sf1()`;
  * `s2` is the result of `sf2()`;
  * `f1` is the result of `DECAY_COPY(std::forward<Function>(f))`;
  * `f2` is a function object of unspecified type that, when called as `f2(i, s2)`, performs `f1(i, s1)`, where `i` is a value of type `size_t`.

```
template<class Function, class ResultFactory, class SharedFactory>
  std::experimental::future<result_of_t<decay_t<ResultFactory>()>>
    void bulk_twoway_execute(Function&& f, size_t n, ResultFactory&& rf, SharedFactory&& sf) const;
```

*Effects:* Performs `e.bulk_twoway_execute(f2, n, rf2, sf2)`, where:
  * `e1` is the target object of `*this`;
  * `e2` is the result of `require(require(e1, bulk), twoway)`;
  * `rf1` is the result of `DECAY_COPY(std::forward<ResultFactory>(rf))`;
  * `rf2` is a function object of unspecified type that, when called as `rf2()`, performs `rf1()`;
  * `sf1` is the result of `DECAY_COPY(std::forward<SharedFactory>(rf))`;
  * `sf2` is a function object of unspecified type that, when called as `sf2()`, performs `sf1()`;
  * if `decltype(rf1())` is non-void, `r1` is the result of `rf1()`;
  * if `decltype(rf2())` is non-void, `r2` is the result of `rf2()`;
  * `s1` is the result of `sf1()`;
  * `s2` is the result of `sf2()`;
  * `f1` is the result of `DECAY_COPY(std::forward<Function>(f))`;
  * if `decltype(rf1())` is non-void and `decltype(rf2())` is non-void, `f2` is a function object of unspecified type that, when called as `f2(i, r2, s2)`, performs `f1(i, r1, s1)`, where `i` is a value of type `size_t`.
  * if `decltype(rf1())` is non-void and `decltype(rf2())` is void, `f2` is a function object of unspecified type that, when called as `f2(i, s2)`, performs `f1(i, r1, s1)`, where `i` is a value of type `size_t`.
  * if `decltype(rf1())` is void and `decltype(rf2())` is non-void, `f2` is a function object of unspecified type that, when called as `f2(i, r2, s2)`, performs `f1(i, s1)`, where `i` is a value of type `size_t`.
  * if `decltype(rf1())` is void and `decltype(rf2())` is void, `f2` is a function object of unspecified type that, when called as `f2(i, s2)`, performs `f1(i, s1)`, where `i` is a value of type `size_t`.

*Returns:* A future, whose shared state is made ready when the future returned by `e.bulk_twoway_execute(f2, n, rf2, sf2)` is made ready, containing the result in `r1` (if `decltype(rf1())` is non-void) or any exception thrown by an invocation`f1`. [*Note:* `e2.bulk_twoway_execute(f2)` may return any future type that satisfies the Future requirements, and not necessarily `std::experimental::future`. One possible implementation approach is for the polymorphic wrapper to attach a continuation to the inner future via that object's `then()` member function. When invoked, this continuation stores the result in the outer future's associated shared and makes that shared state ready. *--end note*]

#### `executor` capacity

```
explicit operator bool() const noexcept;
```

*Returns:* `true` if `*this` has a target, otherwise `false`.

#### `executor` target access

```
const type_info& target_type() const noexcept;
```

*Returns:* If `*this` has a target of type `T`, `typeid(T)`; otherwise, `typeid(void)`.

```
template<class Executor> Executor* target() noexcept;
template<class Executor> const Executor* target() const noexcept;
```

*Returns:* If `target_type() == typeid(Executor)` a pointer to the stored executor target; otherwise a null pointer value.

#### `executor` comparisons

```
bool operator==(const executor& a, const executor& b) noexcept;
```

*Returns:*

- `true` if `!a` and `!b`;
- `true` if `a` and `b` share a target;
- `true` if `e` and `f` are the same type and `e == f`, where `e` is the target of `a` and `f` is the target of `b`;
- otherwise `false`.

```
bool operator==(const executor& e, nullptr_t) noexcept;
bool operator==(nullptr_t, const executor& e) noexcept;
```

*Returns:* `!e`.

```
bool operator!=(const executor& a, const executor& b) noexcept;
```

*Returns:* `!(a == b)`.

```
bool operator!=(const executor& e, nullptr_t) noexcept;
bool operator!=(nullptr_t, const executor& e) noexcept;
```

*Returns:* `(bool) e`.

#### `executor` specialized algorithms

```
void swap(executor& a, executor& b) noexcept;
```

*Effects:* `a.swap(b)`.

### Class `executor::context_type`


## Thread pools

Thread pools create execution agents which execute on threads without incurring the
overhead of thread creation and destruction whenever such agents are needed.

### Header `<thread_pool>` synopsis

```
namespace std {
namespace experimental {
inline namespace concurrency_v2 {

  class static_thread_pool;

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std
```

### Class `static_thread_pool`

`static_thread_pool` is a statically-sized thread pool which may be explicitly
grown via thread attachment. The `static_thread_pool` is expected to be created
with the use case clearly in mind with the number of threads known by the
creator. As a result, no default constructor is considered correct for
arbitrary use cases and `static_thread_pool` does not support any form of
automatic resizing.   

`static_thread_pool` presents an effectively unbounded input queue and the execution functions of `static_thread_pool`'s associated executors do not block on this input queue.

[*Note:* Because `static_thread_pool` provides parallel execution agents,
situations which require concurrent execution properties are not guaranteed
correctness. *--end note.*]

```
class static_thread_pool
{
  public:
    class executor_type;
    
    // construction/destruction
    explicit static_thread_pool(std::size_t num_threads);
    
    // nocopy
    static_thread_pool(const static_thread_pool&) = delete;
    static_thread_pool& operator=(const static_thread_pool&) = delete;

    // stop accepting incoming work and wait for work to drain
    ~static_thread_pool();

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

bool operator==(const static_thread_pool& a, const static_thread_pool& b) noexcept;
bool operator!=(const static_thread_pool& a, const static_thread_pool& b) noexcept;
```

The class `static_thread_pool` satisfies the `ExecutionContext` requirements.

For an object of type `static_thread_pool`, *outstanding work* is defined as the sum
of:

* the total number of calls to the `on_work_started` function that returned
  `true`, less the total number of calls to the `on_work_finished` function, on
  any executor associated with the `static_thread_pool`.

* the number of function objects that have been added to the `static_thread_pool`
  via the `static_thread_pool` executor, but not yet executed; and

* the number of function objects that are currently being executed by the
  `static_thread_pool`.

The `static_thread_pool` member functions `executor`, `attach`, `wait`, and `stop`,
and the `static_thread_pool::executor_type` copy constructors and member functions, do
not introduce data races as a result of concurrent calls to those functions
from different threads of execution.

#### Construction and destruction

```
static_thread_pool(std::size_t num_threads);
```

*Effects:* Constructs a `static_thread_pool` object with `num_threads` threads of
execution, as if by creating objects of type `std::thread`.

```
~static_thread_pool();
```

*Effects:* Destroys an object of class `static_thread_pool`. Performs `stop()`
followed by `wait()`.

#### Worker Management

```
void attach();
```

*Effects:* Adds the calling thread to the pool such that this thread is used to
execute submitted function objects. (Note: Threads created during thread pool
construction, or previously attached to the pool, will continue to be used for
function object execution. --end note) Blocks the calling thread until
signalled to complete by `stop()` or `wait()`, and then blocks until all the
threads created during `static_thread_pool` object construction have completed.
(NAMING: a possible alternate name for this function is `join()`.)

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

#### Executor Creation

```
executor_type executor() noexcept;
```

*Returns:* An executor that may be used to submit function objects to the
thread pool.

#### Comparisons

```
bool operator==(const static_thread_pool& a, const static_thread_pool& b) noexcept;
```

*Returns:* `std::addressof(a) == std::addressof(b)`.

```
bool operator!=(const static_thread_pool& a, const static_thread_pool& b) noexcept;
```

*Returns:* `!(a == b)`.

### Class `static_thread_pool::executor_type`

```
class static_thread_pool::executor_type
{
  public:
    // types:

    typedef bulk_parallel_execution bulk_forward_progress_guarantee;
    typedef possibly_blocking_execution blocking_guarantee;

    typedef std::size_t shape_type;
    typedef std::size_t index_type;

    // construct / copy / destroy:

    executor_type(const executor_type& other) noexcept;
    executor_type(executor_type&& other) noexcept;

    executor_type& operator=(const executor_type& other) noexcept;
    executor_type& operator=(executor_type&& other) noexcept;

    // executor operations:

    bool running_in_this_thread() const noexcept;

    static_thread_pool& context() const noexcept;

    bool on_work_started() const noexcept;
    void on_work_finished() const noexcept;
};

bool operator==(const static_thread_pool::executor_type& a,
                const static_thread_pool::executor_type& b) noexcept;
bool operator!=(const static_thread_pool::executor_type& a,
                const static_thread_pool::executor_type& b) noexcept;
```

`static_thread_pool::executor_type` is a type satisfying the `BaseExecutor` and
`ExecutorWorkTracker` requirements. Objects of type
`static_thread_pool::executor` are associated with a `static_thread_pool`.

The customization points `execute`, `post`, `defer`, `sync_execute`,
`async_execute`, `async_post`, `async_defer`, `then_execute`, `bulk_execute`,
`bulk_post`, `bulk_defer`, `bulk_sync_execute`, `bulk_async_execute`,
`bulk_async_post`, `bulk_async_defer`, and `bulk_then_execute` are well-formed
for this executor. Function objects submitted using these customization points
will be executed by the `static_thread_pool`.

For the customization points `execute`, `sync_execute`, `async_execute`,
`bulk_execute`, `bulk_sync_execute`, and `bulk_async_execute`, if
`running_in_this_thread()` is `true`, calls at least one of the submitted
function objects in the current thread prior to returning from the
customization point. [*Note:* If this function object exits via an exception,
the exception propagates to the caller. *--end note*]

#### Constructors

```
executor_type(const executor_type& other) noexcept;
```

*Postconditions:* `*this == other`.

```
executor_type(executor_type&& other) noexcept;
```

*Postconditions:* `*this` is equal to the prior value of `other`.

#### Assignment

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

#### Operations

```
bool running_in_this_thread() const noexcept;
```

*Returns:* `true` if the current thread of execution is a thread that was
created by or attached to the associated `static_thread_pool` object.

```
static_thread_pool& context() const noexcept;
```

*Returns:* A reference to the associated `static_thread_pool` object.

```
bool on_work_started() const noexcept;
```

*Effects:* Increments the count of outstanding work associated with the
`static_thread_pool`.

*Returns:* `false` if there was a prior call to the `stop()` member function of
the associated `static_thread_pool` object; otherwise `true`.

```
void on_work_finished() const noexcept;
```

*Effects:* Decrements the count of outstanding work associated with the
`static_thread_pool`.

#### Comparisons

```
bool operator==(const static_thread_pool::executor_type& a,
                const static_thread_pool::executor_type& b) noexcept;
```

*Returns:* `a.context() == b.context()`.

```
bool operator!=(const static_thread_pool::executor_type& a,
                const static_thread_pool::executor_type& b) noexcept;
```

*Returns:* `!(a == b)`.
