### Header `<execution>` synopsis

```
namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

  // Associated execution context property:
  struct context_t;

  constexpr context_t context;

  // Directionality properties:

  struct oneway_t;
  struct twoway_t;
  struct then_t;

  constexpr oneway_t oneway;
  constexpr twoway_t twoway;
  constexpr then_t then;

  // Cardinality properties:

  struct single_t;
  struct bulk_t;

  constexpr single_t single;
  constexpr bulk_t bulk;

  // Blocking properties:

  struct possibly_blocking_t;
  struct always_blocking_t;
  struct never_blocking_t;

  constexpr possibly_blocking_t possibly_blocking;
  constexpr always_blocking_t always_blocking;
  constexpr never_blocking_t never_blocking;

  // Properties to allow adaptation of blocking and directionality:

  struct adaptable_blocking_t;
  struct not_adaptable_blocking_t;

  constexpr adaptable_blocking_t adaptable_blocking;
  constexpr not_adaptable_blocking_t not_adaptable_blocking;

  // Properties to indicate if submitted tasks represent continuations:

  struct not_continuation_t;
  struct continuation_t;

  constexpr not_continuation_t not_continuation;
  constexpr continuation_t continuation;

  // Properties to indicate likely task submission in the future:

  struct not_outstanding_work_t;
  struct outstanding_work_t;

  constexpr not_outstanding_work_t not_outstanding_work;
  constexpr outstanding_work_t outstanding_work;

  // Properties for bulk execution guarantees:

  struct bulk_sequenced_execution_t;
  struct bulk_parallel_execution_t;
  struct bulk_unsequenced_execution_t;

  constexpr bulk_sequenced_execution_t bulk_sequenced_execution;
  constexpr bulk_parallel_execution_t bulk_parallel_execution;
  constexpr bulk_unsequenced_execution_t bulk_unsequenced_execution;

  // Properties for mapping of execution on to threads:

  struct other_execution_mapping_t;
  struct thread_execution_mapping_t;
  struct new_thread_execution_mapping_t;

  constexpr other_execution_mapping_t other_execution_mapping;
  constexpr thread_execution_mapping_t thread_execution_mapping;
  constexpr new_thread_execution_mapping_t new_thread_execution_mapping;

  // Memory allocation properties:

  template<class ProtoAllocator> struct allocator_t { ProtoAllocator a; };
  template<> struct allocator_t<void> {
    template<class ProtoAllocator>
    constexpr allocator_t<ProtoAllocator> operator()(const ProtoAllocator& a) const {
      return allocator_t<ProtoAllocator>{a};
    }
  };
  constexpr allocator_t<void> allocator;

  // Executor type traits:

  template<class Executor> struct is_oneway_executor;
  template<class Executor> struct is_twoway_executor;
  template<class Executor> struct is_then_executor;
  template<class Executor> struct is_bulk_oneway_executor;
  template<class Executor> struct is_bulk_twoway_executor;
  template<class Executor> struct is_bulk_then_executor;

  template<class Executor> constexpr bool is_oneway_executor_v = is_oneway_executor<Executor>::value;
  template<class Executor> constexpr bool is_twoway_executor_v = is_twoway_executor<Executor>::value;
  template<class Executor> constexpr bool is_then_executor_v = is_then_executor<Executor>::value;
  template<class Executor> constexpr bool is_bulk_oneway_executor_v = is_bulk_oneway_executor<Executor>::value;
  template<class Executor> constexpr bool is_bulk_twoway_executor_v = is_bulk_twoway_executor<Executor>::value;
  template<class Executor> constexpr bool is_bulk_then_executor_v = is_bulk_then_executor<Executor>::value;

  template<class Executor, class T> struct executor_future;
  template<class Executor> struct executor_shape;
  template<class Executor> struct executor_index;

  template<class Executor, class T> using executor_future_t = typename executor_future<Executor, T>::type;
  template<class Executor> using executor_shape_t = typename executor_shape<Executor>::type;
  template<class Executor> using executor_index_t = typename executor_index<Executor>::type;

  // Member detection type traits for properties:

  template<class Executor, class Property> struct has_require_member;
  template<class Executor, class Property> struct has_query_member;

  template<class Executor, class Property>
    constexpr bool has_require_member_v = has_require_member<Executor, Property>::value;
  template<class Executor, class Property>
    constexpr bool has_query_member_v = has_query_member<Executor, Property>::value;

  // Member return type traits for properties:

  template<class Executor, class Property> struct require_member_result;
  template<class Executor, class Property> struct query_member_result;

  template<class Executor, class Property>
    using require_member_result_t = typename require_member_result<Executor, Property>::type;
  template<class Executor, class Property>
    using query_member_result_t = typename query_member_result<Executor, Property>::type;

  // Property value types traits:

  template<class Executor, class Property> struct property_value;

  template<class Executor, class Property>
    using property_value_t = typename property_value<Executor, Property>::type;

  // Customization points:

  namespace {
    constexpr unspecified require = unspecified;
    constexpr unspecified prefer = unspecified;
    constexpr unspecified query = unspecified;
  }

  // Customization point type traits:

  template<class Executor, class... Properties> struct can_require;
  template<class Executor, class... Properties> struct can_prefer;
  template<class Executor, class Property> struct can_query;

  template<class Executor, class... Properties>
    constexpr bool can_require_v = can_require<Executor, Properties...>::value;
  template<class Executor, class... Properties>
    constexpr bool can_prefer_v = can_prefer<Executor, Properties...>::value;
  template<class Executor, class Property>
    constexpr bool can_query_v = can_query<Executor, Property>::value;

  // Polymorphic executor wrappers:

  class bad_executor;
  class executor;

} // namespace execution
} // inline namespace executors_v1
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

### General requirements on executors

An executor type `X` shall satisfy the requirements of `CopyConstructible` (C++Std [copyconstructible]), `Destructible` (C++Std [destructible]), and `EqualityComparable` (C++Std [equalitycomparable]), as well as the additional requirements listed below.

No comparison operator, copy operation, move operation, swap operation, or member function `context` on these types shall exit via an exception.

The executor copy constructor, comparison operators, associated execution functions, associated query functions, and other member functions defined in executor type requirements shall not introduce data races as a result of concurrent calls to those functions from different threads.

The destructor shall not block pending completion of the submitted function objects. [*Note:* The ability to wait for completion of submitted function objects may be provided by the associated execution context. *--end note*]

In the Table \ref{base_executor_requirements} below, `x1` and `x2` denote (possibly const) values of some executor type `X`, `mx1` denotes an xvalue of type `X`, and `u` denotes an identifier.

Table: (General executor requirements) \label{base_executor_requirements}

| Expression   | Type       | Assertion/note/pre-/post-condition |
|--------------|------------|------------------------------------|
| `X u(x1);` | | Shall not exit via an exception. <br/><br/>*Post:* `u == x1`. |
| `X u(mx1);` | | Shall not exit via an exception. <br/><br/>*Post:* `u` equals the prior value of `mx1`. |
| `x1 == x2` | `bool` | Returns `true` only if `x1` and `x2` can be interchanged with identical effects in any of the expressions defined in these type requirements (TODO and the other executor requirements defined in this Technical Specification). [*Note:* Returning `false` does not necessarily imply that the effects are not identical. *--end note*] `operator==` shall be reflexive, symmetric, and transitive, and shall not exit via an exception. |
| `x1 != x2` | `bool` | Same as `!(x1 == x2)`. |

[*Commentary: The equality operator is specified primarily as an aid to support postconditons on executor copy construction and move construction. The key word in supporting these postconditions is "interchanged". That is, if a copy is substituted for the original executor it shall produce identical effects, provided the expression, calling context, and program state are otherwise identical. Calls to the copied executor from a different context or program state are not required to produce identical effects, and this is not considered an "interchanged" use of an executor. In particular, even consecutive calls to the same executor need not produce identical effects since the program state has already altered.*]

### `OneWayExecutor` requirements

The `OneWayExecutor` requirements specify requirements for executors which create execution agents without a channel for awaiting the completion of a submitted function object and obtaining its result. [*Note:* That is, the executor provides fire-and-forget semantics. *--end note*]

A type `X` satisfies the `OneWayExecutor` requirements if it satisfies the general requirements on executors, as well as the requirements in the table below.

In the Table below, `x` denotes a (possibly const) executor object of type `X` and `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))()` and where `decay_t<F>` satisfies the `MoveConstructible` requirements.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.execute(f)` | `void` | Creates an execution agent which invokes `DECAY_COPY( std::forward<F>(f))()` at most once, with the call to `DECAY_COPY` being evaluated in the thread that called `execute`. <br/> <br/> May block forward progress of the caller until `DECAY_COPY( std::forward<F>(f))()` finishes execution. <br/> <br/> The invocation of `execute` synchronizes with (C++Std [intro.multithread]) the invocation of `f`. <br/> <br/> `execute` shall not propagate any exception thrown by `DECAY_COPY( std::forward<F>(f))()` or any other function submitted to the executor. [*Note:* The treatment of exceptions thrown by one-way submitted functions is specific to the concrete executor type. *--end note.*] |

### `TwoWayExecutor` requirements

The `TwoWayExecutor` requirements specify requirements for executors which creating execution agents with a channel for awaiting the completion of a submitted function object and obtaining its result.

A type `X` satisfies the `TwoWayExecutor` requirements if it satisfies the general requirements on executors, as well as the requirements in the table below.

In the Table below, `x` denotes a (possibly const) executor object of type `X`, `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))()` and where `decay_t<F>` satisfies the `MoveConstructible` requirements, and `R` denotes the type of the expression `DECAY_COPY(std::forward<F>(f))()`.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.twoway_execute(f)` | A type that satisfies the `Future` requirements for the value type `R`. | Creates an execution agent which invokes `DECAY_COPY( std::forward<F>(f))()` at most once, with the call to `DECAY_COPY` being evaluated in the thread that called `twoway_execute`. <br/> <br/> May block forward progress of the caller until `DECAY_COPY( std::forward<F>(f))()` finishes execution. <br/> <br/> The invocation of `twoway_execute` synchronizes with (C++Std [intro.multithread]) the invocation of `f`. <br/> <br/> Stores the result of `DECAY_COPY( std::forward<F>(f))()`, or any exception thrown by `DECAY_COPY( std::forward<F>(f))()`, in the associated shared state of the resulting `Future`. |

### `ThenExecutor` requirements

The `ThenExecutor` requirements specify requirements for executors which create execution agents whose initiation is predicated on the readiness of a specified future, and which provide a channel for awaiting the completion of the submitted function object and obtaining its result.

A type `X` satisfies the `ThenExecutor` requirements if it satisfies the general requirements on executors, as well as the requirements in the table below.

In the Table below, `x` denotes a (possibly const) executor object of type `X`, `fut` denotes a future object satisfying the Future requirements, `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))(fut)` and where `decay_t<F>` satisfies the `MoveConstructible` requirements, and `R` denotes the type of the expression `DECAY_COPY(std::forward<F>(f))(fut)`.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.then_execute(f, fut)` | A type that satisfies the `Future` requirements for the value type `R`. | When `fut` is ready, creates an execution agent which invokes `DECAY_COPY( std::forward<F>(f))(fut)` at most once, with the call to `DECAY_COPY` being evaluated in the thread that called `then_execute`. <br/> <br/> May block forward progress of the caller until `DECAY_COPY( std::forward<F>(f))(fut)` finishes execution. <br/> <br/> The invocation of `then_execute` synchronizes with (C++Std [intro.multithread]) the invocation of `f`. <br/> <br/> Stores the result of `DECAY_COPY( std::forward<F>(f))(fut)`, or any exception thrown by `DECAY_COPY( std::forward<F>(f))(fut)`, in the associated shared state of the resulting `Future`. |

### `BulkOneWayExecutor` requirements

The `BulkOneWayExecutor` requirements specify requirements for executors which create groups of execution agents in bulk from a single execution function, without a channel for awaiting the completion of the submitted function object invocations and obtaining their result. [*Note:* That is, the executor provides fire-and-forget semantics. *--end note*]

A type `X` satisfies the `BulkOneWayExecutor` requirements if it satisfies the general requirements on executors, as well as the requirements in the table below.

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

A type `X` satisfies the `BulkTwoWayExecutor` requirements if it satisfies the general requirements on executors, as well as the requirements in the table below.

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

### `BulkThenExecutor` requirements

The `BulkThenExecutor` requirements specify requirements for executors which create execution agents whose initiation is predicated on the readiness of a specified future, and which provide a channel for awaiting the completion of the submitted function object and obtaining its result.

A type `X` satisfies the `BulkThenExecutor` requirements if it satisfies the general requirements on executors, as well as the requirements in the table below.

In the Table below,

  * `x` denotes a (possibly const) executor object of type `X`,
  * `n` denotes a shape object whose type is `executor_shape_t<X>`,
  * `fut` denotes a future object satisfying the Future requirements,
  * `rf` denotes a `CopyConstructible` function object with zero arguments whose result type is `R`,
  * `sf` denotes a `CopyConstructible` function object with zero arguments whose result type is `S`,
  * `i` denotes a (possibly const) object whose type is `executor_index_t<X>`,
  * `s` denotes an object whose type is `S`,
  * if `R` is non-void,
    * `r` denotes an object whose type is `R`,
    * `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))(i, fut, r, s)` and where `decay_t<F>` satisfies the `MoveConstructible` requirements.
  * if `R` is void,
    * `f` denotes a function object of type `F&&` callable as `DECAY_COPY(std::forward<F>(f))(i, fut, s)` and where `decay_t<F>` satisfies the `MoveConstructible` requirements.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `x.bulk_then_execute(f, n, fut, rf, sf)` | A type that satisfies the `Future` requirements for the value type `R`. | If `R` is non-void, invokes `rf()` on an unspecified execution agent to produce the value `r`. Invokes `sf()` on an unspecified execution agent to produce the value `s`. When `fut` is ready, creates a group of execution agents of shape `n` which invokes `DECAY_COPY( std::forward<F>(f))(i, fut, r, s)` if `R` is non-void, and otherwise invokes `DECAY_COPY( std::forward<F>(f))(i, fut, s)`, at most once for each value of `i` in the range `[0,n)`, with the call to `DECAY_COPY` being evaluated in the thread that called `bulk_then_execute`. <br/> <br/> May block forward progress of the caller until one or more invocations of `f` finish execution. <br/> <br/> The invocation of `bulk_then_execute` synchronizes with (C++Std [intro.multithread]) the invocations of `f`. <br/> <br/> Once all invocations of `f` finish execution, stores `r`, or any exception thrown by an invocation of `f`, in the associated shared state of the resulting `Future`. |

## Executor properties

### In general

An executor's behavior in generic contexts is determined by a set of executor properties, and each executor property imposes certain requirements on the executor.

Given an existing executor, a related executor with different properties may be created by calling the `require` member or non-member functions. These functions behave according the table below. In the table below, `x` denotes a (possibly const) executor object of type `X`, and `p` denotes a (possibly const) property object.

[*Note:* As a general design note properties which define a mutually exclusive pair, that describe an enabled or non-enabled behaviour follow the convention of having the same property name for both with the `not_` prefix to the property for the non-enabled behaviour. *--end note*]

| Expression | Comments |
|------------|----------|
| `x.require(p)` <br/> `require(x,p)` | Returns an executor object with the requested property `p` added to the set. All other properties of the returned executor are identical to those of `x`, except where those properties are described below as being mutually exclusive to `p`. In this case, the mutually exclusive properties are implicitly removed from the set associated with the returned executor. <br/> <br/> The expression is ill formed if an executor is unable to add the requested property. |

The current value of an executor's properties can be queried by calling the `query` function. This function behaves according the table below. In the table below, `x` denotes a (possibly const) executor object of type `X`, and `p` denotes a (possibly const) property object.

| Expression | Comments |
|------------|----------|
| `x.query(p)` | Returns the current value of the requested property `p`. The expression is ill formed if an executor is unable to return the requested property. |

### Query-only properties

#### Associated execution context property

    struct context_t
    {
      static constexpr bool is_requirable = true;
      static constexpr bool is_preferable = false;

      using polymorphic_query_result_type = any; // TODO: alternatively consider void*, or simply omitting the type.

      template<class Executor>
        static constexpr bool is_supportable
          = can_query_v<Executor, context_t>;
            // TODO consider requiring the query result to be an lvalue reference.
    };

The `context_t` property can be used only with `query`, which returns the **execution context** associated with the executor.

An execution context is a program object that represents a specific collection of execution resources and the **execution agents** that exist within those resources. Execution agents are units of execution, and a 1-to-1 mapping exists between an execution agent and an invocation of a callable function object submitted via the executor.

### Interface-changing properties

#### Directionality properties

    struct oneway_t;
    struct twoway_t;
    struct then_t;

The directionality properties conform to the following specification:

    struct S
    {
      static constexpr bool is_requirable = true;
      static constexpr bool is_preferable = false;

      using polymorphic_query_result_type = bool;

      template<class Executor>
        static constexpr bool is_supportable
          = see-below;
    };

| Property | Requirements | is_supportable                 |
|----------|--------------| -------------------------------|
| `oneway_t` | The executor type satisfies the `OneWayExecutor` or `BulkOneWayExecutor` requirements. | `can_query_v<Executor, S> && (is_oneway_executor_v<Executor> || is_bulk_oneway_executor_v<Executor>)` |
| `twoway_t` | The executor type satisfies the `TwoWayExecutor` or `BulkTwoWayExecutor` requirements. | `can_query_v<Executor, S> && (is_twoway_executor_v<Executor> || is_bulk_twoway_executor_v<Executor>)` |
| `then_t` | The executor type satisfies the `ThenExecutor` or `BulkThenExecutor` requirements. | `can_query_v<Executor, S> && (is_then_executor_v<Executor> || is_bulk_then_executor_v<Executor>)` |

The `oneway_t`, `twoway_t` and `then_t` properties are not mutually exclusive.

#### Cardinality properties

    struct single_t;
    struct bulk_t;

The cardinality properties conform to the following specification:

    struct S
    {
      static constexpr bool is_requirable = true;
      static constexpr bool is_preferable = false;

      using polymorphic_query_result_type = bool;

      template<class Executor>
        static constexpr bool is_supportable
          = see-below;
    };

| Property | Requirements | is_supportable                      |
|----------|--------------| ------------------------------------|
| `single_t` | The executor type satisfies the `OneWayExecutor`, `TwoWayExecutor`, or `ThenExecutor` requirements. | `can_query_v<Executor, S> && (is_oneway_executor_v<Executor> || is_twoway_executor_v<Executor> || is_then_executor_v<Executor>)` |
| `bulk_t` | The executor type satisfies the `BulkOneWayExecutor`, `BulkTwoWayExecutor`, or `BulkThenExecutor` requirements. | `can_query_v<Executor, S> && (is_bulk_oneway_executor_v<Executor> || is_bulk_twoway_executor_v<Executor> || is_bulk_then_executor_v<Executor>)` |

The `single_t` and `bulk_t` properties are not mutually exclusive.

### Behavioral properties

Unless otherwise specified, behavioral properties conform to the following specification:

    struct S
    {
      static constexpr bool is_requirable = true;
      static constexpr bool is_preferable = true;

      using polymorphic_query_result_type = bool;

      template<class Executor>
        static constexpr bool is_supportable
          = can_query_v<Executor, S>;
    };

Unless otherwise specified, the result of the `query` customization point for a
behavioral property is `true` if the property is present in the executor, and
`false` if it is not.

#### Blocking properties

    struct possibly_blocking_t;
    struct always_blocking_t;
    struct never_blocking_t;

| Property | Requirements |
|----------|--------------|
| `possibly_blocking_t` | A call to an executor's execution function may block pending completion of one or more of the execution agents created by that execution function. |
| `always_blocking_t` | A call to an executor's execution function shall block until completion of all execution agents created by that execution function. |
| `never_blocking_t` | A call to an executor's execution function shall not block pending completion of the execution agents created by that execution function. |

The `possibly_blocking_t`, `always_blocking_t` and `never_blocking_t` properties are mutually exclusive.

[*Note:* The guarantees of `possibly_blocking_t`, `always_blocking_t` and `never_blocking_t` implies the relationships: `possibly_blocking < always_blocking` and `possibly_blocking < never_blocking` *--end note*]

#### Properties to indicate if blocking and directionality may be adapted

    struct adaptable_blocking_t;
    struct not_adaptable_blocking_t;

| Property | Requirements |
|----------|--------------|
| `adaptable_blocking_t` | The `require` customization point may adapt the executor to add the `two_way_t` or `always_blocking_t` properties. |
| `not_adaptable_blocking_t` | The `require` customization point may not adapt the executor to add the `two_way_t` or `always_blocking_t` properties. |

The `not_adaptable_blocking_t` and `adaptable_blocking_t` properties are mutually exclusive.

[*Note:* The `two_way_t` property is included here as the `require` customization point's `two_way_t` adaptation is specified in terms of `std::experimental::future`, and that template supports blocking wait operations. *--end note*]

#### Properties to indicate if submitted tasks represent continuations

    struct continuation_t;
    struct not_continuation_t;

| Property | Requirements |
|----------|--------------|
| `continuation_t` | Function objects submitted through the executor represent continuations of the caller. If the caller is a lightweight execution agent managed by the executor or its associated execution context, the execution of the submitted function object may be deferred until the caller completes. |
| `not_continuation_t` | Function objects submitted through the executor do not represent continuations of the caller. |

The `not_continuation_t` and `continuation_t` properties are mutually exclusive.

#### Properties to indicate likely task submission in the future

    struct not_outstanding_work_t;
    struct outstanding_work_t;

| Property | Requirements |
|----------|--------------|
| `outstanding_work_t` | The existence of the executor object represents an indication of likely future submission of a function object. The executor or its associated execution context may choose to maintain execution resources in anticipation of this submission. |
| `not_outstanding_work_t` | The existence of the executor object does not indicate any likely future submission of a function object. |

The `not_outstanding_work_t` and `outstanding_work_t` properties are mutually exclusive.

[*Note:* The `outstanding_work_t` and `not_outstanding_work_t` properties are use to communicate to the associated execution context intended future work submission on the executor. The intended effect of the properties is the behavior of execution context's facilities for awaiting outstanding work; specifically whether it considers the existance of the executor object with the `outstanding_work_t` property enabled outstanding work when deciding what to wait on. However this will be largely defined by the execution context implementation. It is intended that the execution context will define its wait facilities and on-destruction behaviour and provide an interface for querying this. An initial work towards this is included in P0737r0. *--end note*]

#### Properties for bulk execution guarantees

These properties communicate the forward progress and ordering guarantees of execution agents with respect to other agents within the same bulk submission.

    struct bulk_sequenced_execution_t;
    struct bulk_parallel_execution_t;
    struct bulk_unsequenced_execution_t;

| Property | Requirements |
|----------|--------------|
| `bulk_sequenced_execution_t` | Execution agents within the same bulk execution may not be parallelized. |
| `bulk_parallel_execution_t` | Execution agents within the same bulk execution may be parallelized. |
| `bulk_unsequenced_execution_t` | Execution agents within the same bulk execution may be parallelized and vectorized. |

Execution agents created by executors with the `bulk_sequenced_execution_t` property execute in sequence in lexicographic order of their indices.

Execution agents created by executors with the `bulk_parallel_execution_t` property execute with a parallel forward progress guarantee. Any such agents executing in the same thread of execution are indeterminately sequenced with respect to each other. [*Note:* It is the caller's responsibility to ensure that the invocation does not introduce data races or deadlocks. *--end note*]

Execution agents created by executors with the `bulk_unsequenced_execution_t` property may execute in an unordered fashion. Any such agents executing in the same thread of execution are unsequenced with respect to each other. [*Note:* This means that multiple execution agents may be interleaved on a single thread of execution, which overrides the usual guarantee from [intro.execution] that function executions do not interleave with one another. *--end note*]

[*Editorial note:* The descriptions of these properties were ported from [algorithms.parallel.user]. The intention is that a future standard will specify execution policy behavior in terms of the fundamental properties of their associated executors. We did not include the accompanying code examples from [algorithms.parallel.user] because the examples seem easier to understand when illustrated by `std::for_each`. *--end editorial note*]

[*Note:* The guarantees provided by these properties implies the relationship: `bulk_unsequenced_execution < bulk_parallel_execution < bulk_sequenced_execution`. *--end note*]

[*Editorial note:* The note above is intended to describe when one bulk executor can be substituted for another and provide the required semantics. For example, if a user needs sequenced execution, then only an executor with the `bulk_sequenced_execution_t` property will do. On the other hand, if a user only needs `bulk_unsequenced_execution_t`, then an executor with any of the above properties will suffice. *--end editorial note*]

The `bulk_unsequenced_execution_t`, `bulk_parallel_execution_t`, and `bulk_sequenced_execution_t` properties are mutually exclusive.

#### Properties for mapping of execution on to threads

    struct other_execution_mapping_t;
    struct thread_execution_mapping_t;
    struct new_thread_execution_mapping_t;

| Property | Requirements |
|----------|--------------|
| `other_execution_mapping_t` | Mapping of each execution agent created by the executor is implementation defined. |
| `thread_execution_mapping_t` | Execution agents created by the executor are mapped onto threads of execution. |
| `new_thread_execution_mapping_t` | Each execution agent created by the executor is mapped onto a new thread of execution. |

The `other_execution_mapping_t`, `thread_execution_mapping_t` and `new_thread_execution_mapping_t` properties are mutually exclusive.

[*Note:* The guarantees of `other_execution_mapping_t`, `thread_execution_mapping_t` and `new_thread_execution_mapping_t` implies the relationship: `other_execution_mapping_t < thread_execution_mapping_t < new_thread_execution_mapping_t` *--end note*]

[*Note:* A mapping of an execution agent onto a thread of execution implies the
agent executes as-if on a `std::thread`. Therefore, the facilities provided by
`std::thread`, such as thread-local storage, are available.
`new_thread_execution_mapping_t` provides stronger guarantees, in
particular that thread-local storage will not be shared between execution
agents. *--end note*]

#### Properties for customizing memory allocation

  template<class ProtoAllocator> struct allocator_t { ProtoAllocator a; };
  template<> struct allocator_t<void> {
    template<class ProtoAllocator>
    constexpr allocator_t<ProtoAllocator> operator()(const ProtoAllocator& a) const {
      return allocator_t<ProtoAllocator>{a};
    }
  };
  constexpr allocator_t<void> allocator;

| Property | Requirements |
|----------|--------------|
| `default_allocator` | Executor implementations shall use a default implementation defined allocator to allocate any memory required to store the submitted function object. |
| `allocator(ProtoAllocator)` | Executor implementations shall use the supplied allocator to allocate any memory required to store the submitted function object. |

[*Note:* As the `allocator(ProtoAllocator)` property requires a value to be provided when being set, it is required to be implemented such that it is callable with the `ProtoAllocator` parameter when used in `require` or `prefer` where the customization points accepts the result of the function call operator of `allocator_t<void>`; `allocator_t<ProtoAllocator>` and is passable as an instance when used in `query` where the customization point accepts an instance of `allocator_t<void>`. *--end note*]

[*Note:* It is permitted for an allocator provided via the `allocator(ProtoAllocator)` property to be the same type as the allocator provided by the `default_allocator` property. *--end note*]

## Executor type traits

### Determining that a type satisfies executor type requirements

    template<class T> struct is_oneway_executor;
    template<class T> struct is_twoway_executor;
    template<class T> struct is_then_executor;
    template<class T> struct is_bulk_oneway_executor;
    template<class T> struct is_bulk_twoway_executor;
    template<class T> struct is_bulk_then_executor;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class T>` <br/>`struct is_oneway_executor` | `T` meets the syntactic requirements for `OneWayExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_twoway_executor` | `T` meets the syntactic requirements for `TwoWayExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_then_executor` | `T` meets the syntactic requirements for `ThenExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_bulk_oneway_executor` | `T` meets the syntactic requirements for `BulkOneWayExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_bulk_twoway_executor` | `T` meets the syntactic requirements for `BulkTwoWayExecutor`. | `T` is a complete type. |
| `template<class T>` <br/>`struct is_bulk_then_executor` | `T` meets the syntactic requirements for `BulkThenExecutor`. | `T` is a complete type. |

### Associated future type

    template<class Executor, class T>
    struct executor_future
    {
      using type = decltype(execution::require(declval<const Executor&>(), execution::twoway).twoway_execute(declval<T(*)()>()));
    };

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
          size_t, helper, decltype(execution::require(declval<const Executor&>(), execution::bulk))
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
          executor_shape_t<Executor>, helper, decltype(execution::require(declval<const Executor&>(), execution::bulk))
        >;

        // exposition only
        static_assert(std::is_integral_v<type>, "index type must be an integral type");
    };

### Member detection type traits for properties

    template<class Executor, class Property> struct has_require_member;
    template<class Executor, class Property> struct has_query_member;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class Executor, class Property>` <br/>`struct has_require_member` | The expression `declval<const Executor>().require( declval<Property>())` is well formed. | `Executor` is a complete type. |
| `template<class Executor, class Property>` <br/>`struct has_query_member` | The expression `declval<const Executor>().query( declval<Property>())` is well formed. | `Executor` is a complete type. |

### Member return type traits for properties

    template<class Executor, class Property> struct require_member_result;
    template<class Executor, class Property> struct query_member_result;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a TransformationTrait (C++Std [meta.rqmts]).

| Template                   | Condition           | Comments  |
|----------------------------|---------------------|-----------|
| `template<class Executor, class Property>` <br/>`struct require_member_result` | The expression `declval<const Executor>().require( declval<Property>())` is well formed. | The member typedef `type` shall name the type of the expression `declval<const Executor>().require( declval<Property())`. |
| `template<class Executor, class Property>` <br/>`struct query_member_result` | The expression `declval<const Executor>().query( declval<Property>())` is well formed. | The member typedef `type` shall name the type of the expression `declval<const Executor>().query( declval<Property())`. |

## Property value types traits for properties

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a TransformationTrait (C++Std [meta.rqmts]).

| Template                   | Condition           | Comments  |
|----------------------------|---------------------|-----------|
| `template<class Executor, class Property>` <br/>`struct property_value` | | The member typedef `type` shall name the type of the value associated with the property `Property`. |

## Executor customization points

*Executor customization points* are functions which adapt an executor's properties. Executor customization points enable uniform use of executors in generic contexts.

When an executor customization point named *NAME* invokes a free execution function of the same name, overload resolution is performed in a context that includes the declaration `void` *NAME*`(auto&... args) = delete;`, where `sizeof...(args)` is the arity of the free execution function. This context also does not include a declaration of the executor customization point.

[*Note:* This provision allows executor customization points to call the executor's free, non-member execution function of the same name without recursion. *--end note*]

Whenever `std::experimental::executors_v1::execution::`*NAME*`(`*ARGS*`)` is a valid expression, that expression satisfies the syntactic requirements for the free execution function named *NAME* with arity `sizeof...(`*ARGS*`)` with that free execution function's semantics.

### `require`

    namespace {
      constexpr unspecified require = unspecified;
    }

The name `require` denotes a customization point. The effect of the expression `std::experimental::executors_v1::execution::require(E, P0, Pn...)` for some expressions `E` and `P0`, and where `Pn...` represents `N` expressions (where `N` is 0 or more), is equivalent to:

* `(E).require(P0)` if `N == 0` and `has_require_member_v<decay_t<decltype(E)>, decltype(P0)>` is true.

* Otherwise, `require(E, P0)` if `N == 0` and the expression is well formed.

* Otherwise, `E` if `N == 0` and:
  * `is_same<decay_t<decltype(P0)>, oneway_t>` is true and `(is_oneway_executor_v<decay_t<decltype(E)>> || is_bulk_oneway_executor_v<decay_t<decltype(E)>>)` is true; or
  * `is_same<decay_t<decltype(P0)>, twoway_t>` is true and `(is_twoway_executor_v<decay_t<decltype(E)>> || is_bulk_twoway_executor_v<decay_t<decltype(E)>>)` is true; or
  * `is_same<decay_t<decltype(P0)>, single_t>` is true and `(is_oneway_executor_v<decay_t<decltype(E)>> || is_twoway_executor_v<decay_t<decltype(E)>>)` is true; or
  * `is_same<decay_t<decltype(P0)>, bulk_t>` is true and `(is_bulk_oneway_executor_v<decay_t<decltype(E)>> || is_bulk_twoway_executor_v<decay_t<decltype(E)>>)` is true; or
  * `is_same<decay_t<decltype(P0)>, possibly_blocking_t>` is true.

* Otherwise, a value `E1` of unspecified type that holds a copy of `E` if `N == 0`, `is_same<decay_t<decltype(P0)>, twoway_t>` is true, and `can_query_v<decltype(E), adaptable_blocking_t>` is true. If `E` satisfies the `OneWayExecutor` requirements, `E1` shall satisfy the `OneWayExecutor` requirements by providing member functions `require`, `query`, and `execute` that forward to the corresponding member functions of the copy of `E`, if present, and `E1` shall satisfy the `TwoWayExecutor` requirements by implementing `twoway_execute` in terms of `execute`. Similarly, if `E` satisfies the `BulkOneWayExecutor` requirements, `E1` shall satisfy the `BulkOneWayExecutor` requirements by providing member functions `require`, `query`, and `bulk_execute` that forward to the corresponding member functions of the copy of `E`, if present, and `E1` shall satisfy the `BulkTwoWayExecutor` requirements by implementing `bulk_twoway_execute` in terms of `bulk_execute`. For some type `T`, the type yielded by `executor_future_t<decltype(E1), T>` is `std::experimental::future<T>`. `E1` has the same executor properties as `E`, except for the addition of the `twoway_t` property.

* Otherwise, a value `E1` of unspecified type that holds a copy of `E` if `N == 0` and `is_same<decay_t<decltype(P0)>, bulk_t>` is true. If `E` satisfies the `OneWayExecutor` requirements, `E1` shall satisfy the `OneWayExecutor` requirements by providing member functions `require`, `query`, and `execute` that forward to the corresponding member functions of the copy of `E`, if present, and `E1` shall satisfy the `BulkExecutor` requirements by implementing `bulk_execute` in terms of `execute`. If `E` also satisfies the `TwoWayExecutor` requirements, `E1` shall satisfy the `TwoWayExecutor` requirements by providing the member function `twoway_execute` that forwards to the corresponding member function of the copy of `E`, if present, and `E1` shall satisfy the `BulkTwoWayExecutor` requirements by implementing `bulk_twoway_execute` in terms of `bulk_execute`. `E1` has the same executor properties as `E`, except for the addition of the `bulk_t` property.

* Otherwise, a value `E1` of unspecified type that holds a copy of `E` if `N == 0`, `is_same<decay_t<decltype(P0)>, always_blocking_t>` is true, and `can_query_v<decltype(E), adaptable_blocking_t>` is true. If `E` satisfies the `OneWayExecutor` requirements, `E1` shall satisfy the `OneWayExecutor` requirements by providing member functions `require`, `query`, and `execute` that forward to the corresponding member functions of the copy of `E`. If `E` satisfies the `TwoWayExecutor` requirements, `E1` shall satisfy the `TwoWayExecutor` requirements by providing member functions `require`, `query`, and `twoway_execute` that forward to the corresponding member functions of the copy of `E`. If `E` satisfies the `BulkOneWayExecutor` requirements, `E1` shall satisfy the `BulkOneWayExecutor` requirements by providing member functions `require`, `query`, and `bulk_execute` that forward to the corresponding member functions of the copy of `E`. If `E` satisfies the `BulkTwoWayExecutor` requirements, `E1` shall satisfy the `BulkTwoWayExecutor` requirements by providing member functions `require`, `query`, and `bulk_twoway_execute` that forward to the corresponding member functions of the copy of `E`. In addition, `E1` provides an overload of `require` such that `E1.require(always_blocking)` returns a copy of `E1`, and all functions `execute`, `twoway_execute`, `bulk_execute`, and `bulk_twoway_execute` shall block the calling thread until the submitted functions have finished execution. `E1` has the same executor properties as `E`, except for the addition of the `always_blocking_t` property, and removal of `never_blocking_t` and `possibly_blocking_t` properties if present.

* Otherwise, a value `E1` of unspecified type that holds a copy of `E` if `N == 0` and `is_same<decay_t<decltype(P0)>, adaptable_blocking_t>` is true. If `E` satisfies the `OneWayExecutor` requirements, `E1` shall satisfy the `OneWayExecutor` requirements by providing member functions `require`, `query`, and `execute` that forward to the corresponding member functions of the copy of `E`. If `E` satisfies the `TwoWayExecutor` requirements, `E1` shall satisfy the `TwoWayExecutor` requirements by providing member functions `require`, `query`, and `twoway_execute` that forward to the corresponding member functions of the copy of `E`. If `E` satisfies the `BulkOneWayExecutor` requirements, `E1` shall satisfy the `BulkOneWayExecutor` requirements by providing member functions `require`, `query`, and `bulk_execute` that forward to the corresponding member functions of the copy of `E`. If `E` satisfies the `BulkTwoWayExecutor` requirements, `E1` shall satisfy the `BulkTwoWayExecutor` requirements by providing member functions `require`, `query`, and `bulk_twoway_execute` that forward to the corresponding member functions of the copy of `E`. In addition, `can_query_v<decltype(E1), adaptable_blocking_t>` is true, and `E1.require(not_adaptable_blocking)` yields a copy of `E`. `E1` has the same executor properties as `E`, except for the addition of the `adaptable_blocking_t` property.

* Otherwise, `std::experimental::executors_v1::execution::require( std::experimental::executors_v1::execution::require(E, P0), Pn...)` if `N > 0` and the expression is well formed.

* Otherwise, `std::experimental::executors_v1::execution::require(E, P0, Pn...)` is ill-formed.

### `prefer`

    namespace {
      constexpr unspecified prefer = unspecified;
    }

The name `prefer` denotes a customization point. The effect of the expression `std::experimental::executors_v1::execution::prefer(E, P0, Pn...)` for some expressions `E` and `P0`, and where `Pn...` represents `N` expressions (where `N` is 0 or more), is equivalent to:

* `(E).require(P0)` if `N == 0`, `P0::is_preferable == true`, and `has_require_member_v<decay_t<decltype(E)>, decltype(P0)>` is true.

* Otherwise, `prefer(E, P0)` if `N == 0`, `P0::is_preferable == true`, and the expression is well formed.

* Otherwise, `E` if `N == 0` and `P0::is_preferable == true`.

* Otherwise, `std::experimental::executors_v1::execution::prefer( std::experimental::executors_v1::execution::prefer(E, P0), Pn...)` if the expression is well formed.

* Otherwise, `std::experimental::executors_v1::execution::prefer(E, P0, Pn...)` is ill-formed.

### `query`

    namespace {
      constexpr unspecified query = unspecified;
    }

The name `query` denotes a customization point. The effect of the expression `std::experimental::executors_v1::execution::query(E, P)` for some expressions `E` and `P` is equivalent to:

* `(E).query(P)` if `has_query_member_v<decay_t<decltype(E)>, decltype(P)>` is true.

* Otherwise, `query(E, P)` if the expression is well formed.

* Otherwise, `std::experimental::executors_v1::execution::query(E, P)` is ill-formed.

### Customization point type traits

    template<class Executor, class... Properties> struct can_require;
    template<class Executor, class... Properties> struct can_prefer;
    template<class Executor, class Property> struct can_query;

This sub-clause contains templates that may be used to query the properties of a type at compile time. Each of these templates is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class T>` <br/>`struct can_require` | The expression `std::experimental::executors_v1::execution::require( declval<const Executor>(), declval<Properties>()...)` is well formed. | `T` is a complete type. |
| `template<class T>` <br/>`struct can_prefer` | The expression `std::experimental::executors_v1::execution::prefer( declval<const Executor>(), declval<Properties>()...)` is well formed. | `T` is a complete type. |
| `template<class T>` <br/>`struct can_query` | The expression `std::experimental::executors_v1::execution::query( declval<const Executor>(), declval<Property>())` is well formed. | `T` is a complete type. |

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

  const context_type& query(context_t) const noexcept;

  executor require(oneway_t) const;
  executor require(twoway_t) const;
  executor require(single_t) const;
  executor require(bulk_t) const;
  executor require(thread_execution_mapping_t) const;
  executor require(never_blocking_t p) const;
  executor require(possibly_blocking_t p) const;
  executor require(always_blocking_t p) const;

  template<class Function>
    void execute(Function&& f) const;

  template<class Function>
    std::experimental::future<result_of_t<decay_t<Function>()>>
      twoway_execute(Function&& f) const

  template<class Function, class SharedFactory>
    void bulk_execute(Function&& f, size_t n, SharedFactory&& sf) const;

  template<class Function, class ResultFactory, class SharedFactory>
    std::experimental::future<result_of_t<decay_t<ResultFactory>()>>
      bulk_twoway_execute(Function&& f, size_t n, ResultFactory&& rf, SharedFactory&& sf) const;

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

executor prefer(const executor& e, continuation_t p);
executor prefer(const executor& e, not_continuation_t p);
executor prefer(const executor& e, outstanding_work_t p);
executor prefer(const executor& e, not_outstanding_work_t p);
executor prefer(const executor& e, bulk_sequenced_execution_t p);
executor prefer(const executor& e, bulk_parallel_execution_t p);
executor prefer(const executor& e, bulk_unsequenced_execution_t p);
executor prefer(const executor& e, new_thread_execution_mapping_t p);
```

The `executor` class satisfies the general requirements on executors.

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
  * `can_prefer_v<Executor, continuation>`
  * `can_prefer_v<Executor, not_continuation>`
  * `can_prefer_v<Executor, outstanding_work>`
  * `can_prefer_v<Executor, not_outstanding_work>`
  * `can_prefer_v<Executor, bulk_sequenced_execution>`
  * `can_prefer_v<Executor, bulk_parallel_execution>`
  * `can_prefer_v<Executor, bulk_unsequenced_execution>`
  * `can_prefer_v<Executor, thread_execution_mapping>`
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
const context_type& query(context_t) const noexcept;
```

*Requires:* `*this != nullptr`.

*Returns:* A polymorphic execution context wrapper whose target is `e.query(execution::context)`, where `e` is the target object of `*this`.

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

```
executor prefer(const executor& e, continuation_t p);
executor prefer(const executor& e, not_continuation_t p);
executor prefer(const executor& e, outstanding_work_t p);
executor prefer(const executor& e, not_outstanding_work_t p);
executor prefer(const executor& e, bulk_sequenced_execution_t p);
executor prefer(const executor& e, bulk_parallel_execution_t p);
executor prefer(const executor& e, bulk_unsequenced_execution_t p);
executor prefer(const executor& e, new_thread_execution_mapping_t p);
```

*Returns:* A polymorphic wrapper whose target is `execution::prefer(e1, p)`, where `e1` is the target object of `e`.

### Class `executor::context_type`

The `executor::context_type` class provides a polymorphic wrapper for the execution context associated with a polymorphic executor.

```
class executor::context_type
{
public:
  context_type(const context_type&) = delete;
  context_type& operator=(const context_type&) = delete;
};

// executor context_type comparisons:

bool operator==(const executor::context_type& a, const executor::context_type& b) noexcept;
template<class Context> operator==(const executor::context_type& a, const Context& b) noexcept;
template<class Context> operator==(const Context& a, const executor::context_type& b) noexcept;
bool operator!=(const executor::context_type& a, const executor::context_type& b) noexcept;
template<class Context> operator!=(const executor::context_type& a, const Context& b) noexcept;
template<class Context> operator!=(const Context& a, const executor::context_type& b) noexcept;
```

The *target* is the execution context that is associated with the target of the `executor` that created the `context_type` wrapper.

#### `executor::context_type` comparisons

```
bool operator==(const executor::context_type& a, const executor::context_type& b) noexcept;
```

*Returns:*
- `true` if `!a` and `!b`;
- `true` if `a` and `b` share a target;
- `true` if `c` and `d` are the same type and `c == d`, where `c` is the target of `a` and `d` is the target of `b`;
- otherwise `false`.

```
template<class Context> operator==(const executor::context_type& a, const Context& b) noexcept;
```

*Returns:*
- `true` if `c` and `b` are the same type and `c == b`, where `c` is the target of `a`;
- otherwise `false`.

```
template<class Context> operator==(const Context& a, const executor::context_type& b) noexcept;
```

*Returns:* `b == a`.

```
bool operator!=(const executor::context_type& a, const executor::context_type& b) noexcept;
template<class Context> operator!=(const executor::context_type& a, const Context& b) noexcept;
template<class Context> operator!=(const Context& a, const executor::context_type& b) noexcept;
```

*Returns:* `!(a == b)`.

## Thread pools

Thread pools create execution agents which execute on threads without incurring the
overhead of thread creation and destruction whenever such agents are needed.

### Header `<thread_pool>` synopsis

```
namespace std {
namespace experimental {
inline namespace executors_v1 {

  class static_thread_pool;

} // inline namespace executors_v1
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
    using executor_type = see-below;
    
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
```

For an object of type `static_thread_pool`, *outstanding work* is defined as the sum
of:

* the number of existing executor objects associated with the
  `static_thread_pool` for which the `execution::outstanding_work` property is
  established;

* the number of function objects that have been added to the `static_thread_pool`
  via the `static_thread_pool` executor, but not yet executed; and

* the number of function objects that are currently being executed by the
  `static_thread_pool`.

The `static_thread_pool` member functions `executor`, `attach`, `wait`, and
`stop`, and the associated executors' copy constructors and member functions,
do not introduce data races as a result of concurrent calls to those
functions from different threads of execution.

#### Types

```
using executor_type = see-below;
```

An executor type conforming to the specification for `static_thread_pool` executor types described below.

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

#### Worker management

```
void attach();
```

*Effects:* Adds the calling thread to the pool such that this thread is used to
execute submitted function objects. [*Note:* Threads created during thread pool
construction, or previously attached to the pool, will continue to be used for
function object execution. *--end note*] Blocks the calling thread until
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

#### Executor creation

```
executor_type executor() noexcept;
```

*Returns:* An executor that may be used to submit function objects to the
thread pool. The returned executor has the following properties already
established:

  * `execution::oneway`
  * `execution::twoway`
  * `execution::then`
  * `execution::single`
  * `execution::bulk`
  * `execution::possibly_blocking`
  * `execution::not_continuation`
  * `execution::not_outstanding_work`
  * `execution::allocator(std::allocator<void>())`

### `static_thread_pool` executor types

All executor types accessible through `static_thread_pool::executor()`, and subsequent calls to the member function `require`, conform to the following specification.

```
class C
{
  public:
    // types:

    typedef std::size_t shape_type;
    typedef std::size_t index_type;

    // construct / copy / destroy:

    C(const C& other) noexcept;
    C(C&& other) noexcept;

    C& operator=(const C& other) noexcept;
    C& operator=(C&& other) noexcept;

    // executor operations:

    C require(execution::oneway_t) const;
    C require(execution::twoway_t) const;
    C require(execution::then_t) const;
    C require(execution::single_t) const;
    C require(execution::bulk_t) const;
    C require(execution::bulk_parallel_execution_t) const;
    C require(execution::thread_execution_mapping_t) const;
    see-below require(execution::never_blocking_t) const;
    see-below require(execution::possibly_blocking_t) const;
    see-below require(execution::always_blocking_t) const;
    see-below require(execution::continuation_t) const;
    see-below require(execution::not_continuation_t) const;
    see-below require(execution::outstanding_work_t) const;
    see-below require(execution::not_outstanding_work_t) const;
    template<class ProtoAllocator>
    see-below require(const execution::allocator_t<ProtoAllocator>& a) const;

    see-below query(execution::context_t) const noexcept;
    see-below query(execution::allocator_t) const noexcept;

    bool running_in_this_thread() const noexcept;

    template<class Function>
      void execute(Function&& f) const;

    template<class Function>
      std::experimental::future<result_of_t<decay_t<Function>()>>
        twoway_execute(Function&& f) const

    template<class Function, class Future>
      std::experimental::future<result_of_t<decay_t<Function>(decay_t<Future>)>>
        then_execute(Function&& f, Future&& pred) const;

    template<class Function, class SharedFactory>
      void bulk_execute(Function&& f, size_t n, SharedFactory&& sf) const;

    template<class Function, class ResultFactory, class SharedFactory>
      std::experimental::future<result_of_t<decay_t<ResultFactory>()>>
        void bulk_twoway_execute(Function&& f, size_t n, ResultFactory&& rf, SharedFactory&& sf) const;
};

bool operator==(const C& a, const C& b) noexcept;
bool operator!=(const C& a, const C& b) noexcept;
```

`C` is a type satisfying the `OneWayExecutor`,
`TwoWayExecutor`, `BulkOneWayExecutor`, and `BulkTwoWayExecutor` requirements.
Objects of type `C` are associated with a `static_thread_pool`.

#### Constructors

```
C(const C& other) noexcept;
```

*Postconditions:* `*this == other`.

```
C(C&& other) noexcept;
```

*Postconditions:* `*this` is equal to the prior value of `other`.

#### Assignment

```
C& operator=(const C& other) noexcept;
```

*Postconditions:* `*this == other`.

*Returns:* `*this`.

```
C& operator=(C&& other) noexcept;
```

*Postconditions:* `*this` is equal to the prior value of `other`.

*Returns:* `*this`.

#### Operations

```
C require(execution::oneway_t) const;
C require(execution::twoway_t) const;
C require(execution::then_t) const;
C require(execution::single_t) const;
C require(execution::bulk_t) const;
C require(execution::bulk_parallel_execution_t) const;
C require(execution::thread_execution_mapping_t) const;
```

*Returns:* `*this`.

```
see-below require(execution::never_blocking_t) const;
see-below require(execution::possibly_blocking_t) const;
see-below require(execution::always_blocking_t) const;
see-below require(execution::continuation_t) const;
see-below require(execution::not_continuation_t) const;
see-below require(execution::outstanding_work_t) const;
see-below require(execution::not_outstanding_work_t) const;
```

*Returns:* An executor object of an unspecified type conforming to these
specifications, associated with the same thread pool as `*this`, and having the
requested property established. When the requested property is part of a group
that is defined as a mutually exclusive set, any other properties in the group
are removed from the returned executor object. All other properties of the
returned executor object are identical to those of `*this`.

```
template<class ProtoAllocator>
  see-below require(const execution::allocator_t<ProtoAllocator>& a) const;
```

*Returns:* An executor object of an unspecified type conforming to these
specifications, associated with the same thread pool as `*this`, with the
`execution::allocator_t<ProtoAllocator>` property established such that
allocation and deallocation associated with function submission will be
performed using a copy of `a.alloc`. All other properties of the returned
executor object are identical to those of `*this`.

```
static_thread_pool& query(execution::context_t) const noexcept;
```

*Returns:* A reference to the associated `static_thread_pool` object.

```
see-below query(execution::allocator_t) const noexcept;
```

*Returns:* The allocator object associated with the executor, with type and
value as previously established by the `execution::allocator_wrapper_t`
property.

```
bool running_in_this_thread() const noexcept;
```

*Returns:* `true` if the current thread of execution is a thread that was
created by or attached to the associated `static_thread_pool` object.

```
template<class Function>
  void execute(Function&& f) const;
```

*Effects:* Submits the function `f` for execution on the `static_thread_pool`
according to the `OneWayExecutor` requirements and the properties established
for `*this`. If the submitted function `f` exits via an exception, the
`static_thread_pool` calls `std::terminate()`.

```
template<class Function>
  std::experimental::future<result_of_t<decay_t<Function>()>>
    twoway_execute(Function&& f) const
```

*Effects:* Submits the function `f` for execution on the `static_thread_pool`
according to the `TwoWayExecutor` requirements and the properties established
for `*this`.

*Returns:* A future with behavior as specified by the `TwoWayExecutor` requirements.

```
template<class Function, class Future>
  std::experimental::future<result_of_t<decay_t<Function>(decay_t<Future>)>>
    then_execute(Function&& f, Future&& pred) const
```

*Effects:* Submits the function `f` for execution on the `static_thread_pool`
according to the `ThenExecutor` requirements and the properties established
for `*this`.

*Returns:* A future with behavior as specified by the `ThenExecutor` requirements.

```
template<class Function, class SharedFactory>
  void bulk_execute(Function&& f, size_t n, SharedFactory&& sf) const;
```

*Effects:* Submits the function `f` for bulk execution on the
`static_thread_pool` according to the `BulkOneWayExecutor` requirements and the
properties established for `*this`. If the submitted function `f` exits via an
exception, the `static_thread_pool` calls `std::terminate()`.

```
template<class Function, class ResultFactory, class SharedFactory>
  std::experimental::future<result_of_t<decay_t<ResultFactory>()>>
    void bulk_twoway_execute(Function&& f, size_t n, ResultFactory&& rf, SharedFactory&& sf) const;
```

*Effects:* Submits the function `f` for bulk execution on the `static_thread_pool`
according to the `BulkTwoWayExecutor` requirements and the properties established
for `*this`.

*Returns:* A future with behavior as specified by the `BulkTwoWayExecutor` requirements.

```
template<class Function, class Future, class ResultFactory, class SharedFactory>
  std::experimental::future<result_of_t<decay_t<ResultFactory>()>>
    void bulk_then_execute(Function&& f, size_t n, Future&& pred, ResultFactory&& rf, SharedFactory&& sf) const;
```

*Effects:* Submits the function `f` for bulk execution on the `static_thread_pool`
according to the `BulkThenExecutor` requirements and the properties established
for `*this`.

*Returns:* A future with behavior as specified by the `BulkThenExecutor` requirements.

#### Comparisons

```
bool operator==(const C& a, const C& b) noexcept;
```

*Returns:* `true` if `&a.query(execution::context) == &b.query(execution::context)` and `a` and `b` have identical
properties, otherwise `false`.

```
bool operator!=(const C& a, const C& b) noexcept;
```

*Returns:* `!(a == b)`.
