## Execution Support Library

### General

This Clause describes components supporting execution of function objects [function.objects].

*(The following definition appears in working draft N4762 [thread.req.lockable.general])*

> An *execution agent* is an entity such as a thread that may perform work in parallel with other execution agents. [*Note:* Implementations or users may introduce other kinds of agents such as processes or thread-pool tasks. *--end note*] The calling agent is determined by context; e.g., the calling thread that contains the call, and so on. 

An execution agent invokes a function object  within an *execution context* such as the calling thread or thread-pool.  An *executor* submits a function object to an execution context to be invoked by an execution agent within that execution context. [*Note:* Invocation of the function object may be inlined such as when the execution context is the calling thread, or may be scheduled such as when the execution context is a thread-pool with task scheduler. *--end note*] An executor may submit a function object with *execution properties* that specify how the submission and invocation of the function object interacts with the submitting thread and execution context, including forward progress guarantees [intro.progress]. 

For the intent of this library and extensions to this library, the *lifetime of an execution agent* begins before the function object is invoked and ends after this invocation completes, either normally or having thrown an exception.


### Header `<execution>` synopsis

```
namespace std {
namespace execution {

  // Customization points:

  inline namespace unspecified{
    inline constexpr unspecified set_value = unspecified;

    inline constexpr unspecified set_done = unspecified;

    inline constexpr unspecified set_error = unspecified;

    inline constexpr unspecified execute = unspecified;

    inline constexpr unspecified submit = unspecified;

    inline constexpr unspecified schedule = unspecified;

    inline constexpr unspecified bulk_execute = unspecified;
  }

  // Concepts:

  template<class T, class E = exception_ptr>
    concept receiver = see-below;

  template<class T, class... An>
    concept receiver_of = see-below;

  template<class S>
    concept sender = see-below;

  template<class S, class R>
    concept sender_to = see-below;

  template<class S>
    concept scheduler = see-below;

  template<class E, class F = void(*)()>
    concept executor = see-below;

  // A no-op receiver type
  class sink_receiver;

  // Indication of executor property applicability
  template<class T> struct is_executor;

  template<class T> constexpr inline bool is_executor_v = is_executor<T>::value;

  // Associated execution context property:

  struct context_t;

  constexpr context_t context;

  // Blocking properties:

  struct blocking_t;

  constexpr blocking_t blocking;

  // Properties to allow adaptation of blocking and directionality:

  struct blocking_adaptation_t;

  constexpr blocking_adaptation_t blocking_adaptation;

  // Properties to indicate if submitted tasks represent continuations:

  struct relationship_t;

  constexpr relationship_t relationship;

  // Properties to indicate likely task submission in the future:

  struct outstanding_work_t;

  constexpr outstanding_work_t outstanding_work;

  // Properties for bulk execution guarantees:

  struct bulk_guarantee_t;

  constexpr bulk_guarantee_t bulk_guarantee;

  // Properties for mapping of execution on to threads:

  struct mapping_t;

  constexpr mapping_t mapping;

  // One-way exception handling property:

  // XXX bikeshed this name. "handler" may imply that the value of the property is some kind of function
  // [*Editorial Note:* The presence of this property seems at-odds with the error-handling scheme adopted from P1660. Does this property need to be eliminated? *--end editorial note.*]
  struct oneway_exception_handler_t;

  constexpr oneway_exception_handler_t oneway_exception_handler;

  // Memory allocation properties:

  template <typename ProtoAllocator>
  struct allocator_t;

  constexpr allocator_t<void> allocator;

  // Executor type traits:

  template<class Executor> struct executor_shape;
  template<class Executor> struct executor_index;

  template<class Executor> using executor_shape_t = typename executor_shape<Executor>::type;
  template<class Executor> using executor_index_t = typename executor_index<Executor>::type;

  // Polymorphic executor support:

  class bad_executor;

  template <class... SupportableProperties> class any_executor;

  template<class Property> struct prefer_only;

} // namespace execution
} // namespace std
```

## Requirements

### `ProtoAllocator` requirements

A type `A` meets the `ProtoAllocator` requirements if `A` is `CopyConstructible` (C++Std [copyconstructible]), `Destructible` (C++Std [destructible]), and `allocator_traits<A>::rebind_alloc<U>` meets the allocator requirements (C++Std [allocator.requirements]), where `U` is an object type. [*Note:* For example, `std::allocator<void>` meets the proto-allocator requirements but not the allocator requirements. *--end note*] No comparison operator, copy operation, move operation, or swap operation on these types shall exit via an exception.

### Customization points

#### `execution::set_value`

The name `execution::set_value` denotes a customization point object. The expression `execution::set_value(R, Vs...)` for some subexpressions `R` and `Vs...` is expression-equivalent to:

- `R.set_value(Vs...)`, if that expression is valid. If the function selected does not send the value(s) `Vs...` to the receiver `R`'s value channel, the program is ill-formed with no diagnostic required.

- Otherwise, `set_value(R, Vs...)`, if that expression is valid, with overload resolution performed in a context that includes the declaration

        void set_value();

    and that does not include a declaration of `execution::set_value`. If the function selected by overload resolution does not send the value(s) `Vs...` to the receiver `R`'s value channel, the program is ill-formed with no diagnostic required.

- Otherwise, `execution::set_value(R, Vs...)` is ill-formed.

[*Editorial note:* We should probably define what "send the value(s) `Vs...` to the receiver `R`'s value channel" means more carefully. *--end editorial note*]

[*Editorial note:* This specification is adapted from `ranges::iter_swap`. *--end editorial note*]

#### `execution::set_done`

The name `execution::set_done` denotes a customization point object. The expression `execution::set_done(R)` for some subexpression `R` is expression-equivalent to:

- `R.set_done()`, if that expression is valid. If the function selected does not signal the receiver `R`'s done channel, the program is ill-formed with no diagnostic required.

- Otherwise, `set_done(R)`, if that expression is valid, with overload resolution performed in a context that includes the declaration

        void set_done();

    and that does not include a declaration of `execution::set_done`. If the function selected by overload resolution does not signal the receiver `R`'s done channel, the program is ill-formed with no diagnostic required.

- Otherwise, `execution::set_done(R)` is ill-formed.

[*Editorial note:* We should probably define what "signal receiver `R`'s done channel" means more carefully. *--end editorial note*]

[*Editorial note:* This specification is adapted from `ranges::iter_swap`. *--end editorial note*]

#### `execution::set_error`

The name `execution::set_error` denotes a customization point object. The expression `execution::set_error(R, E)` for some subexpressions `R` and `E` are expression-equivalent to:

- `R.set_error(E)`, if that expression is valid. If the function selected does not send the error `E` to the receiver `R`'s error channel, the program is ill-formed with no diagnostic required.

- Otherwise, `set_error(R, E)`, if that expression is valid, with overload resolution performed in a context that includes the declaration

        void set_error();

    and that does not include a declaration of `execution::set_error`. If the function selected by overload resolution does not send the error `E` to the receiver `R`'s error channel, the program is ill-formed with no diagnostic required.

- Otherwise, `execution::set_error(R, E)` is ill-formed.

[*Editorial note:* We should probably define what "send the error `E` to the receiver `R`'s error channel" means more carefully. *--end editorial note*]

[*Editorial note:* This specification is adapted from `ranges::iter_swap`. *--end editorial note*]

#### `execution::execute`

The name `execution::execute` denotes a customization point object. The expression `execution::execute(E, F)` for some subexpressions `E` and `F` is expression-equivalent to:

- `E.execute(F)`, if that expression is valid. If the function selected does not execute the function object `F` on the executor `E`, the program is ill-formed with no diagnostic required.

- Otherwise, `execute(E, F)`, if that expression is valid, with overload resolution performed in a context that includes the declaration

        void execute();

    and that does not include a declaration of `execution::execute`. If the function selected by overload resolution does not execute the function object `F` on the executor `E`, the program is ill-formed with no diagnostic required.

- Otherwise, if the type `F` models `invocable`, then `execution::submit(E, execution::make_callback_from(F))`.

- Otherwise, `execution::execute(E, F)` is ill-formed.

[*Editorial note:* We should probably define what "execute the function object `F` on the executor `E`" means more carefully. *--end editorial note*]

[*Editorial note:* This specification is adapted from `ranges::iter_swap`. *--end editorial note*]

#### `execution::submit`

The name `execution::submit` denotes a customization point object. For some subexpressions `s` and `r`, let `S` be a type such that `decltype((s))` is `S` and let `R` be a type such that `decltype((r))` is `R`. The expression `execution::submit(s, r)` is ill-formed if `R` does not model `receiver`, or if `S` does not model either `sender` or `executor`. Otherwise, it is expression-equivalent to:

- `s.submit(r)`, if that expression is valid and `S` models `sender`. If the function selected does not submit the receiver object `r` via the sender `s`, the program is ill-formed with no diagnostic required.

- Otherwise, `submit(s, r)`, if that expression is valid and `S` models `sender`, with overload resolution performed in a context that includes the declaration

        void submit();

    and that does not include a declaration of `execution::submit`. If the function selected by overload resolution does not submit the receiver object `r` via the sender `s`, the program is ill-formed with no diagnostic required.

- Otherwise, `execution::execute(s, `_`as-invocable`_`<R>(forward<R>(r)))` if `S` and _`as-invocable`_`<R>` model `executor`, where _`as-invocable`_ is some implementation-defined class template equivalent to:

        template<receiver R>
        struct as-invocable {
        private:
          using receiver_type = std::remove_cvref_t<R>
          std::optional<receiver_type> r_ {};
          void try_init_(auto&& r) {
            try {
              r_.emplace((decltype(r)&&) r);
            } catch(...) {
              execution::set_error(r, current_exception());
            }
          }
        public:
          explicit as-invocable(receiver_type&& r) {
            try_init_(move_if_noexcept(r));
          }
          explicit as-invocable(const receiver_type& r) {
            try_init_(r);
          }
          as-invocable(as-invocable&& other) {
            if(other.r_) {
              try_init_(move_if_noexcept(*other.r_));
              other.r_.reset();
            }
          }
          ~as-invocable() {
            if(r_)
              execution::set_done(*r_);
          }
          void operator()() {
            try {
              execution::set_value(*r_);
            } catch(...) {
              execution::set_error(*r_, current_exception());
            }
            r_.reset();
          }
        };

- Otherwise, `execution::submit(s, r)` is ill-formed.

[*Editorial note:* We should probably define what "submit the receiver object `R` via the sender `S`" means more carefully. *--end editorial note*]

[*Editorial note:* This specification is adapted from `ranges::iter_swap`. *--end editorial note*]

#### `execution::schedule`

The name `execution::schedule` denotes a customization point object. The expression `execution::schedule(S)` for some subexpression `S` is expression-equivalent to:

- `S.schedule()`, if that expression is valid and its type `N` models `Sender`. 

- Otherwise, `schedule(S)`, if that expression is valid and its type `N` models `Sender` with overload resolution performed in a context that includes the declaration

        void schedule();

    and that does not include a declaration of `execution::schedule`. 

- Otherwise, _`decay-copy`_`(S)` if the type `S` models `Sender`.

- Otherwise, `execution::schedule(S)` is ill-formed.

[*Editorial note:* This specification is adapted from `ranges::iter_swap`. *--end editorial note*]

#### `execution::bulk_execute`

The name `execution::bulk_execute` denotes a customization point object. If `is_convertible_v<S, executor_shape_t<remove_cvref_t<E>>>` is true, then the expression `execution::bulk_execute(E, F, S)` for some subexpressions `E`, `F`, and `S` is expression-equivalent to:

- `E.bulk_execute(F, S)`, if that expression is valid. If the function selected does not execute `S` invocations of the function object `F` on the executor `E` in bulk, and the result of that function does not model `sender<void>`, the program is ill-formed with no diagnostic required.

- Otherwise, `bulk_execute(E, F, S)`, if that expression is valid, with overload resolution performed in a context that includes the declaration

        void bulk_execute();

    and that does not include a declaration of `execution::bulk_execute`. If the function selected by overload resolution does not execute `S` invocations of the function object `F` on the executor `E` in bulk, and the result of that function does not model `sender<void>`, the program is ill-formed with no diagnostic required.

- Otherwise, if the types `F` and `executor_index_t<remove_cvref_t<E>>` model `invocable`, then `execution::tbd_concrete_implementations::bulk_execute(E, F, S)`.

- Otherwise, `execution::bulk_execute(E, F, S)` is ill-formed.

[*Editorial note:* We should probably define what "execute `S` invocations of the function object `F` on the executor `E` in bulk" means more carefully. *--end editorial note*]

[*Editorial note:* This specification is adapted from `ranges::iter_swap`. *--end editorial note*]

### Concept `receiver`

XXX TODO The `receiver` concept...

```
template<class T, class E = exception_ptr>
concept receiver =
  move_constructible<remove_cvref_t<T>> &&
  requires(T&& t, E&& e) {
    { execution::set_done((T&&) t) } noexcept;
    { execution::set_error((T&&) t, (E&&) e) } noexcept;
  };
```

### Concept `receiver_of`

XXX TODO The `receiver_of` concept...

```
template<class T, class... An>
concept receiver_of =
  receiver<T> &&
  requires(T&& t, An&&... an) {
    execution::set_value((T&&) t, (An&&) an...);
  };
```

### Concept `sender`

XXX TODO The `sender` concept...

```
template<class S, class R>
concept sender-to-impl =
  requires(S&& s, R&& r) {
    execution::submit((S&&) s, (R&&) r);
  };

template<class S>
concept sender =
  sender-to-impl<S, sink_receiver>;
```

### Concept `sender_to`

XXX TODO The `sender_to` concept...

```
template<class S, class R>
concept sender_to =
  sender<S> &&
  receiver<R> &&
  sender-to-impl<S, R>;
```
### Concept `scheduler`

XXX TODO The `scheduler` concept...

```
template<class S>
concept scheduler =
  copy_constructible<remove_cvref_t<S>> &&
  equality_comparable<remove_cvref_t<S>> &&
  requires(E&& e) {
    execution::scheduler((S&&)s);
  } // && sender<invoke_result_t<execution::scheduler, S>>
  };
```

None of a scheduler's copy constructor, destructor, equality comparison, or `swap` operation shall exit via an exception.

None of these operations, nor an scheduler type's `schedule` function, or associated query functions shall introduce data races as a result of concurrent invocations of those functions from different threads.

For any two (possibly const) values `x1` and `x2` of some scheduler type `X`, `x1 == x2` shall return `true` only if `x1.query(p) == x2.query(p)` for every property `p` where both `x1.query(p)` and `x2.query(p)` are well-formed and result in a non-void type that is `EqualityComparable` (C++Std [equalitycomparable]). [*Note:* The above requirements imply that `x1 == x2` returns `true` if `x1` and `x2` can be interchanged with identical effects. An scheduler may conceptually contain additional properties which are not exposed by a named property type that can be observed via `execution::query`; in this case, it is up to the concrete scheduler implementation to decide if these properties affect equality. Returning `false` does not necessarily imply that the effects are not identical. *--end note*]

An scheduler type's destructor shall not block pending completion of any function objects submitted to the returned object that models `Sender`. [*Note:* The ability to wait for completion of submitted function objects may be provided by the execution context that produced the scheduler. *--end note*]

In addition to the above requirements, type `S` models `scheduler` only if it satisfies the requirements from at least one row in the Table below.

In the Table below, 

- `s` denotes a (possibly const) scheduler object of type `S`,
- `N` denotes a type that models `Sender`, and
- `n` denotes a sender object of type `N`

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `execution::schedule(s)` | `N` | Evaluates `execution::schedule(s)` on the calling thread to create `n` |

### Concept `executor`

XXX TODO The `executor` concept...

```
template<class E, class F = void(*)()>
concept executor =
  invocable<F> &&
  copy_constructible<remove_cvref_t<E>> &&
  equality_comparable<remove_cvref_t<E>> &&
  requires(E&& e, F&& f) {
    execution::execute((E&&)e,(F&&)f);
  };
```

None of an executor's copy constructor, destructor, equality comparison, or `swap` operation shall exit via an exception.

None of these operations, nor an executor type's `execute` or `submit` functions, or associated query functions shall introduce data races as a result of concurrent invocations of those functions from different threads.

For any two (possibly const) values `x1` and `x2` of some executor type `X`, `x1 == x2` shall return `true` only if `x1.query(p) == x2.query(p)` for every property `p` where both `x1.query(p)` and `x2.query(p)` are well-formed and result in a non-void type that is `equality_comparable` (C++Std [equalitycomparable]). [*Note:* The above requirements imply that `x1 == x2` returns `true` if `x1` and `x2` can be interchanged with identical effects. An executor may conceptually contain additional properties which are not exposed by a named property type that can be observed via `execution::query`; in this case, it is up to the concrete executor implementation to decide if these properties affect equality. Returning `false` does not necessarily imply that the effects are not identical. *--end note*]

An executor type's destructor shall not block pending completion of the submitted function objects. [*Note:* The ability to wait for completion of submitted function objects may be provided by the associated execution context. *--end note*]

For an executor type `E`, the expression `is_executor_v<E>` shall be a valid constant expression with the value `true`.

In addition to the above requirements, types `E` and `F` model `executor` only if they satisfy the requirements from at least one row in the Table below.

In the Table below, 

- `e` denotes a (possibly const) executor object of type `E`,
- `cf` denotes the function or receiver object `DECAY_COPY(std::forward<F>(f))` 
- `f` denotes a function or receiver object of type `F&&` invocable as `cf()` and where `decay_t<F>` models `move_constructible`.

| Expression | Return Type | Operational semantics |
|------------|-------------|---------------------- |
| `e.execute(f)` | `void` | Evaluates `DECAY_COPY(std::forward<F>(f))` on the calling thread to create `cf` that will be invoked at most once by an execution agent. <br/> May block pending completion of this invocation. <br/> Synchronizes with [intro.multithread] the invocation of `f`. <br/>Shall not propagate any exception thrown by the function object or any other function submitted to the executor. [*Note:* The treatment of exceptions thrown by one-way submitted functions is described by the `execution::oneway_exception_handler` property. The forward progress guarantee of the associated execution agent(s) is implementation defined. *--end note.*] |
| `e.submit(f)` | `void` | Evaluates `DECAY_COPY(std::forward<F>(f))` on the calling thread to create `cf` that will be invoked at most once by an execution agent. <br/> May block pending completion of this invocation. <br/> Synchronizes with [intro.multithread] the invocation of `f`. <br/>Shall not propagate any exception thrown by the function object or any other function submitted to the executor. [*Note:* The treatment of exceptions thrown by one-way submitted functions is described by the `execution::oneway_exception_handler` property. The forward progress guarantee of the associated execution agent(s) is implementation defined. *--end note.*] |

### No-op receiver

XXX TODO `sink_receiver`

```c++
class sink_receiver {
public:
  void set_value(auto&&...) {}
  [[noreturn]] void set_error(auto&&) noexcept {
    std::terminate();
  }
  void set_done() noexcept {}
};
```

### Executor applicability trait

```c++
template<class T> struct is_executor;
```

This sub-clause contains a template that may be used to query the applicability of the properties provided in this clause to a type at compile time.  It may be specialized to indicate applicability of executor properties to a type. This template is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class T>` <br/>`struct is_executor` | The expression `P::is_executor` is a well-formed constant expression with a value of `true`. | `T` is a complete type. |

### Query-only properties

#### Associated execution context property

    struct context_t
    {
      template <class T>
        static constexpr bool is_applicable_property_v = is_executor_v<T>;

      static constexpr bool is_requirable = false;
      static constexpr bool is_preferable = false;
    
      using polymorphic_query_result_type = any;
    
      template<class Executor>
        static constexpr decltype(auto) static_query_v
          = Executor::query(context_t());
    };

The `context_t` property can be used only with `query`, which returns the execution context associated with the executor.

The value returned from `execution::query(e, context_t)`, where `e` is an executor, shall not change between invocations.

#### Polymorphic executor wrappers

The `any_executor` class template provides polymorphic wrappers for executors.

In several places in this section the operation `CONTAINS_PROPERTY(p, pn)` is used. All such uses mean `std::disjunction_v<std::is_same<p, pn>...>`.

In several places in this section the operation `FIND_CONVERTIBLE_PROPERTY(p, pn)` is used. All such uses mean the first type `P` in the parameter pack `pn` for which `std::is_convertible_v<p, P>` is `true`. If no such type `P` exists, the operation `FIND_CONVERTIBLE_PROPERTY(p, pn)` is ill-formed.

```
template <class... SupportableProperties>
class any_executor
{
public:
  // indication of applicability to executor properties
  static constexpr bool is_executor_v = true;

  // construct / copy / destroy:

  any_executor() noexcept;
  any_executor(nullptr_t) noexcept;
  any_executor(const any_executor& e) noexcept;
  any_executor(any_executor&& e) noexcept;
  template<class... OtherSupportableProperties>
    any_executor(any_executor<OtherSupportableProperties...> e);
  template<class... OtherSupportableProperties>
    any_executor(any_executor<OtherSupportableProperties...> e) = delete;
  template<class Executor>
    any_executor(Executor e);

  any_executor& operator=(const any_executor& e) noexcept;
  any_executor& operator=(any_executor&& e) noexcept;
  any_executor& operator=(nullptr_t) noexcept;
  template<class Executor>
    any_executor& operator=(Executor e);

  ~any_executor();

  // any_executor modifiers:

  void swap(any_executor& other) noexcept;

  // any_executor operations:

  template <class Property>
  any_executor require(Property) const;

  template <class Property>
  typename Property::polymorphic_query_result_type query(Property) const;

  template<class Function>
    void execute(Function&& f) const;

  // any_executor capacity:

  explicit operator bool() const noexcept;

  // any_executor target access:

  const type_info& target_type() const noexcept;
  template<class Executor> Executor* target() noexcept;
  template<class Executor> const Executor* target() const noexcept;

  // any_executor casts:

  template<class... OtherSupportableProperties>
    any_executor<OtherSupportableProperties...> static_executor_cast() const;
};

// any_executor comparisons:

template <class... SupportableProperties>
bool operator==(const any_executor<SupportableProperties...>& a, const any_executor<SupportableProperties...>& b) noexcept;
template <class... SupportableProperties>
bool operator==(const any_executor<SupportableProperties...>& e, nullptr_t) noexcept;
template <class... SupportableProperties>
bool operator==(nullptr_t, const any_executor<SupportableProperties...>& e) noexcept;
template <class... SupportableProperties>
bool operator!=(const any_executor<SupportableProperties...>& a, const any_executor<SupportableProperties...>& b) noexcept;
template <class... SupportableProperties>
bool operator!=(const any_executor<SupportableProperties...>& e, nullptr_t) noexcept;
template <class... SupportableProperties>
bool operator!=(nullptr_t, const any_executor<SupportableProperties...>& e) noexcept;

// any_executor specialized algorithms:

template <class... SupportableProperties>
void swap(any_executor<SupportableProperties...>& a, any_executor<SupportableProperties...>& b) noexcept;

template <class Property, class... SupportableProperties>
any_executor prefer(const any_executor<SupportableProperties>& e, Property p);
```

The `any_executor` class satisfies the `executor` concept requirements.

[*Note:* To meet the `noexcept` requirements for executor copy constructors and move constructors, implementations may share a target between two or more `any_executor` objects. *--end note*]

Each property type in the `SupportableProperties...` pack shall provide a nested type `polymorphic_query_result_type`.

The *target* is the executor object that is held by the wrapper.

##### `any_executor` constructors

```
any_executor() noexcept;
```

*Postconditions:* `!*this`.

```
any_executor(nullptr_t) noexcept;
```

*Postconditions:* `!*this`.

```
any_executor(const any_executor& e) noexcept;
```

*Postconditions:* `!*this` if `!e`; otherwise, `*this` targets `e.target()` or a copy of `e.target()`.

```
any_executor(any_executor&& e) noexcept;
```

*Effects:* If `!e`, `*this` has no target; otherwise, moves `e.target()` or move-constructs the target of `e` into the target of `*this`, leaving `e` in a valid state with an unspecified value.

```
template<class... OtherSupportableProperties>
  any_executor(any_executor<OtherSupportableProperties...> e);
```

*Remarks:* This function shall not participate in overload resolution unless:
* `CONTAINS_PROPERTY(p, OtherSupportableProperties)` , where `p` is each property in `SupportableProperties...`.

*Effects:* `*this` targets a copy of `e` initialized with `std::move(e)`.

```
template<class... OtherSupportableProperties>
  any_executor(any_executor<OtherSupportableProperties...> e) = delete;
```

*Remarks:* This function shall not participate in overload resolution unless `CONTAINS_PROPERTY(p, OtherSupportableProperties)` is `false` for some property `p` in `SupportableProperties...`.

```
template<class Executor>
  any_executor(Executor e);
```

*Remarks:* This function shall not participate in overload resolution unless:

* `can_require_v<Executor, P>`, if `P::is_requirable`, where `P` is each property in `SupportableProperties...`.
* `can_prefer_v<Executor, P>`, if `P::is_preferable`, where `P` is each property in `SupportableProperties...`.
* and `can_query_v<Executor, P>`, if `P::is_requirable == false` and `P::is_preferable == false`, where `P` is each property in `SupportableProperties...`.

*Effects:* `*this` targets a copy of `e`.

##### `any_executor` assignment

```
any_executor& operator=(const any_executor& e) noexcept;
```

*Effects:* `any_executor(e).swap(*this)`.

*Returns:* `*this`.

```
any_executor& operator=(any_executor&& e) noexcept;
```

*Effects:* Replaces the target of `*this` with the target of `e`, leaving `e` in a valid state with an unspecified value.

*Returns:* `*this`.

```
any_executor& operator=(nullptr_t) noexcept;
```

*Effects:* `any_executor(nullptr).swap(*this)`.

*Returns:* `*this`.

```
template<class Executor>
  any_executor& operator=(Executor e);
```

*Requires:* As for `template<class Executor> any_executor(Executor e)`.

*Effects:* `any_executor(std::move(e)).swap(*this)`.

*Returns:* `*this`.

##### `any_executor` destructor

```
~any_executor();
```

*Effects:* If `*this != nullptr`, releases shared ownership of, or destroys, the target of `*this`.

##### `any_executor` modifiers

```
void swap(any_executor& other) noexcept;
```

*Effects:* Interchanges the targets of `*this` and `other`.

##### `any_executor` operations

```
template <class Property>
any_executor require(Property p) const;
```

*Remarks:* This function shall not participate in overload resolution unless `FIND_CONVERTIBLE_PROPERTY(Property, SupportableProperties)::is_requirable` is well-formed and has the value `true`.

*Returns:* A polymorphic wrapper whose target is the result of `execution::require(e, p)`, where `e` is the target object of `*this`.

```
template <class Property>
typename Property::polymorphic_query_result_type query(Property p) const;
```

*Remarks:* This function shall not participate in overload resolution unless `FIND_CONVERTIBLE_PROPERTY(Property, SupportableProperties)` is well-formed.

*Returns:* If `execution::query(e, p)` is well-formed, `static_cast<Property::polymorphic_query_result_type>(execution::query(e, p))`, where `e` is the target object of `*this`. Otherwise, `Property::polymorphic_query_result_type{}`.

```
template<class Function>
  void execute(Function&& f) const;
```

*Effects:* Performs `execution::execute(e, f2)`, where:

  * `e` is the target object of `*this`;
  * `f1` is the result of `DECAY_COPY(std::forward<Function>(f))`;
  * `f2` is a function object of unspecified type that, when invoked as `f2()`, performs `f1()`.

##### `any_executor` capacity

```
explicit operator bool() const noexcept;
```

*Returns:* `true` if `*this` has a target, otherwise `false`.

##### `any_executor` target access

```
const type_info& target_type() const noexcept;
```

*Returns:* If `*this` has a target of type `T`, `typeid(T)`; otherwise, `typeid(void)`.

```
template<class Executor> Executor* target() noexcept;
template<class Executor> const Executor* target() const noexcept;
```

*Returns:* If `target_type() == typeid(Executor)` a pointer to the stored executor target; otherwise a null pointer value.

##### `any_executor` comparisons

```
template<class... SupportableProperties>
bool operator==(const any_executor<SupportableProperties...>& a, const any_executor<SupportableProperties...>& b) noexcept;
```

*Returns:*

- `true` if `!a` and `!b`;
- `true` if `a` and `b` share a target;
- `true` if `e` and `f` are the same type and `e == f`, where `e` is the target of `a` and `f` is the target of `b`;
- otherwise `false`.

```
template<class... SupportableProperties>
bool operator==(const any_executor<SupportableProperties...>& e, nullptr_t) noexcept;
template<class... SupportableProperties>
bool operator==(nullptr_t, const any_executor<SupportableProperties...>& e) noexcept;
```

*Returns:* `!e`.

```
template<class... SupportableProperties>
bool operator!=(const any_executor<SupportableProperties...>& a, const any_executor<SupportableProperties...>& b) noexcept;
```

*Returns:* `!(a == b)`.

```
template<class... SupportableProperties>
bool operator!=(const any_executor<SupportableProperties...>& e, nullptr_t) noexcept;
template<class... SupportableProperties>
bool operator!=(nullptr_t, const any_executor<SupportableProperties...>& e) noexcept;
```

*Returns:* `(bool) e`.

##### `any_executor` specialized algorithms

```
template<class... SupportableProperties>
void swap(any_executor<SupportableProperties...>& a, any_executor<SupportableProperties...>& b) noexcept;
```

*Effects:* `a.swap(b)`.

```
template <class Property, class... SupportableProperties>
any_executor prefer(const any_executor<SupportableProperties...>& e, Property p);
```

*Remarks:* This function shall not participate in overload resolution unless `FIND_CONVERTIBLE_PROPERTY(Property, SupportableProperties)::is_preferable` is well-formed and has the value `true`.

*Returns:* A polymorphic wrapper whose target is the result of `execution::prefer(e, p)`, where `e` is the target object of `*this`.

##### `any_executor` casts

```
template<class... OtherSupportableProperties>
  any_executor<OtherSupportableProperties...> static_executor_cast() const;
```

*Requires:* The target object was first inserted into a polymorphic wrapper (whether via the wrapper's constructor or assignment operator) whose template parameters included the parameters in `OtherSupportableProperties`.

*Returns:* A polymorphic wrapper whose target is `e`.

### Behavioral properties

Behavioral properties define a set of mutually-exclusive nested properties describing executor behavior.

Unless otherwise specified, behavioral property types `S`, their nested property types `S::N`*i*, and nested property objects `S::n`*i* conform to the following specification:

    struct S
    {
      template <class T>
        static constexpr bool is_applicable_property_v = is_executor_v<T>;

      static constexpr bool is_requirable = false;
      static constexpr bool is_preferable = false;
      using polymorphic_query_result_type = S;
    
      template<class Executor>
        static constexpr auto static_query_v
          = see-below;
    
      template<class Executor>
      friend constexpr S query(const Executor& ex, const Property& p) noexcept(see-below);
    
      friend constexpr bool operator==(const S& a, const S& b);
      friend constexpr bool operator!=(const S& a, const S& b) { return !operator==(a, b); }
    
      constexpr S();
    
      struct N1
      {
        static constexpr bool is_requirable = true;
        static constexpr bool is_preferable = true;
        using polymorphic_query_result_type = S;
    
        template<class Executor>
          static constexpr auto static_query_v
            = see-below;
    
        static constexpr S value() { return S(N1()); }
      };
    
      static constexpr N1 n1;
    
      constexpr S(const N1);
    
      ...
    
      struct NN
      {
        static constexpr bool is_requirable = true;
        static constexpr bool is_preferable = true;
        using polymorphic_query_result_type = S;
    
        template<class Executor>
          static constexpr auto static_query_v
            = see-below;
    
        static constexpr S value() { return S(NN()); }
      };
    
      static constexpr NN nN;
    
      constexpr S(const NN);
    };

Queries for the value of an executor's behavioral property shall not change between invocations unless the executor is assigned another executor with a different value of that behavioral property.

`S()` and `S(S::E`*i*`())` are all distinct values of `S`. [*Note:* This means they compare unequal. *--end note.*]

The value returned from `execution::query(e1, p1)` and a subsequent invocation `execution::query(e1, p1)`, where

* `p1` is an instance of `S` or `S::E`*i*, and
* `e2` is the result of `std::require(e1, p2)` or `std::prefer(e1, p2)`,

shall compare equal unless

* `p2` is an instance of `S::E`*i*, and
* `p1` and `p2` are different types.

The value of the expression `S::N1::static_query_v<Executor>` is

* `Executor::query(S::N1())`, if that expression is a well-formed expression;
* ill-formed if `declval<Executor>().query(S::N1())` is well-formed;
* ill-formed if `can_query_v<Executor,S::N`*i*`>` is `true` for any `1 < ` *i* `<= N`;
* otherwise `S::N1()`.

[*Note:* These rules automatically enable the `S::N1` property by default for executors which do not provide a `query` function for properties `S::N`*i*. *--end note*]

The value of the expression `S::N`*i*`::static_query_v<Executor>`, for all `1 < ` *i* `<= N`, is

* `Executor::query(S::N`*i*`())`, if that expression is a well-formed constant expression;
* otherwise ill-formed.

The value of the expression `S::static_query_v<Executor>` is

* `Executor::query(S())`, if that expression is a well-formed constant expression;
* otherwise, ill-formed if `declval<Executor>().query(S())` is well-formed;
* otherwise, `S::N`*i*`::static_query_v<Executor>` for the least *i* `<= N` for which this expression is a well-formed constant expression;
* otherwise ill-formed.

[*Note:* These rules automatically enable the `S::N1` property by default for executors which do not provide a `query` function for properties `S` or `S::N`*i*. *--end note*]

Let *k* be the least value of *i* for which `can_query_v<Executor,S::N`*i*`>` is true, if such a value of *i* exists.

```
template<class Executor>
  friend constexpr S query(const Executor& ex, const Property& p) noexcept(noexcept(execution::query(ex, std::declval<const S::Nk>())));
```

*Returns:* `std::query(ex, S::N`*k*`())`.

*Remarks:* This function shall not participate in overload resolution unless `is_same_v<Property,S> && can_query_v<Executor,S::N`*i*`>` is true for at least one `S::N`*i*`. 


```
bool operator==(const S& a, const S& b);
```

*Returns:* `true` if `a` and `b` were constructed from the same constructor; `false`, otherwise.

#### Blocking properties

The `blocking_t` property describes what guarantees executors provide about the blocking behavior of their execution functions.

`blocking_t` provides nested property types and objects as described below.

| Nested Property Type | Nested Property Object Name | Requirements |
|--------------------------|------------------------|--------------|
| `blocking_t::possibly_t` | `blocking.possibly` | Invocation of an executor's execution function may block pending completion of one or more invocations of the submitted function object. |
| `blocking_t::always_t` | `blocking.always` | Invocation of an executor's execution function shall block until completion of all invocations of submitted function object. |
| `blocking_t::never_t` | `blocking.never` | Invocation of an executor's execution function shall not block pending completion of the invocations of the submitted function object. |

##### `blocking_t::always_t` customization points

In addition to conforming to the above specification, the `blocking_t::always_t` property provides the following customization:

    struct always_t
    {
      static constexpr bool is_requirable = true;
      static constexpr bool is_preferable = false;

      template <class T>
        static constexpr bool is_applicable_property_v = is_executor_v<T>;

      template<class Executor>
        friend see-below require(Executor ex, blocking_t::always_t);
    };

If the executor has the `blocking_adaptation_t::allowed_t` property, this customization uses an adapter to implement the `blocking_t::always_t` property.

```
template<class Executor>
  friend see-below require(Executor ex, blocking_t::always_t);
```

*Returns:* A value `e1` of type `E1` that holds a copy of `ex`. `E1` provides an overload of `require` such that `e1.require(blocking.always)` returns a copy of `e1`, an overload of `query` such that `e1.query(blocking)` returns `blocking.always`, and functions `execute` and `bulk_execute` shall block the calling thread until the submitted functions have finished execution. `e1` has the same executor properties as `ex`, except for the addition of the `blocking_t::always_t` property, and removal of `blocking_t::never_t` and `blocking_t::possibly_t` properties if present.

*Remarks:* This function shall not participate in overload resolution unless `blocking_adaptation_t::static_query_v<Executor>` is `blocking_adaptation.allowed`.

#### Properties to indicate if blocking and directionality may be adapted

The `blocking_adaptation_t` property allows or disallows blocking or directionality adaptation via `execution::require`.

`blocking_adaptation_t` provides nested property types and objects as described below.

| Nested Property Type | Nested Property Object Name | Requirements |
|--------------------------|---------------------------------|--------------|
| `blocking_adaptation_t::disallowed_t` | `blocking_adaptation.disallowed` | The `require` customization point may not adapt the executor to add the `blocking_t::always_t` property. |
| `blocking_adaptation_t::allowed_t` | `blocking_adaptation.allowed` | The `require` customization point may adapt the executor to add the `blocking_t::always_t` property. |

##### `blocking_adaptation_t::allowed_t` customization points

In addition to conforming to the above specification, the `blocking_adaptation_t::allowed_t` property provides the following customization:

    struct allowed_t
    {
      static constexpr bool is_requirable = true;
      static constexpr bool is_preferable = false;

      template <class T>
        static constexpr bool is_applicable_property_v = is_executor_v<T>;

      template<class Executor>
        friend see-below require(Executor ex, blocking_adaptation_t::allowed_t);
    };

This customization uses an adapter to implement the `blocking_adaptation_t::allowed_t` property.

```
template<class Executor>
  friend see-below require(Executor ex, blocking_adaptation_t::allowed_t);
```

*Returns:* A value `e1` of type `E1` that holds a copy of `ex`. In addition, `blocking_adaptation_t::static_query_v<E1>` is `blocking_adaptation.allowed`, and `e1.require(blocking_adaptation.disallowed)` yields a copy of `ex`. `e1` has the same executor properties as `ex`, except for the addition of the `blocking_adaptation_t::allowed_t` property.

#### Properties to indicate if submitted tasks represent continuations

The `relationship_t` property allows users of executors to indicate that submitted tasks represent continuations.

`relationship_t` provides nested property types and objects as indicated below.

| Nested Property Type | Nested Property Object Name | Requirements |
|--------------------------|---------------------------------|--------------|
| `relationship_t::fork_t` | `relationship.fork` | Function objects submitted through the executor do not represent continuations of the caller. |
| `relationship_t::continuation_t` | `relationship.continuation` | Function objects submitted through the executor represent continuations of the caller. Invocation of the submitted function object may be deferred until the caller completes. |

#### Properties to indicate likely task submission in the future

The `outstanding_work_t` property allows users of executors to indicate that task submission is likely in the future.

`outstanding_work_t` provides nested property types and objects as indicated below.

| Nested Property Type| Nested Property Object Name | Requirements |
|-------------------------|---------------------------------|--------------|
| `outstanding_work_t::untracked_t` | `outstanding_work.untracked` | The existence of the executor object does not indicate any likely future submission of a function object. |
| `outstanding_work_t::tracked_t` | `outstanding_work.tracked` | The existence of the executor object represents an indication of likely future submission of a function object. The executor or its associated execution context may choose to maintain execution resources in anticipation of this submission. |

[*Note:* The `outstanding_work_t::tracked_t` and `outstanding_work_t::untracked_t` properties are used to communicate to the associated execution context intended future work submission on the executor. The intended effect of the properties is the behavior of execution context's facilities for awaiting outstanding work; specifically whether it considers the existance of the executor object with the `outstanding_work_t::tracked_t` property enabled outstanding work when deciding what to wait on. However this will be largely defined by the execution context implementation. It is intended that the execution context will define its wait facilities and on-destruction behaviour and provide an interface for querying this. An initial work towards this is included in P0737r0. *--end note*]

#### Properties for bulk execution guarantees

Bulk execution guarantee properties communicate the forward progress and ordering guarantees of execution agents associated with the bulk execution.

`bulk_guarantee_t` provides nested property types and objects as indicated below.

| Nested Property Type | Nested Property Object Name | Requirements |
|--------------------------|---------------------------------|--------------|
| `bulk_guarantee_t::unsequenced_t` | `bulk_guarantee.unsequenced` | Execution agents within the same bulk execution may be parallelized and vectorized. |
| `bulk_guarantee_t::sequenced_t` | `bulk_guarantee.sequenced` | Execution agents within the same bulk execution may not be parallelized. |
| `bulk_guarantee_t::parallel_t` | `bulk_guarantee.parallel` | Execution agents within the same bulk execution may be parallelized. |

Execution agents associated with the `bulk_guarantee_t::unsequenced_t` property may invoke the function object in an unordered fashion. Any such invocations in the same thread of execution are unsequenced with respect to each other. [*Note:* This means that multiple execution agents may be interleaved on a single thread of execution, which overrides the usual guarantee from [intro.execution] that function executions do not interleave with one another. *--end note*]

Execution agents associated with the `bulk_guarantee_t::sequenced_t` property invoke the function object in sequence in lexicographic order of their indices.

Execution agents associated with the `bulk_guarantee_t::parallel_t` property invoke the function object with a parallel forward progress guarantee. Any such invocations in the same thread of execution are indeterminately sequenced with respect to each other. [*Note:* It is the caller's responsibility to ensure that the invocation does not introduce data races or deadlocks. *--end note*]

[*Editorial note:* The descriptions of these properties were ported from [algorithms.parallel.user]. The intention is that a future standard will specify execution policy behavior in terms of the fundamental properties of their associated executors. We did not include the accompanying code examples from [algorithms.parallel.user] because the examples seem easier to understand when illustrated by `std::for_each`. *--end editorial note*]

#### Properties for mapping of execution on to threads

The `mapping_t` property describes what guarantees executors provide about the mapping of execution agents onto threads of execution.

`mapping_t` provides nested property types and objects as indicated below.

| Nested Property Type| Nested Property Object Name | Requirements |
|-------------------------|---------------------------------|--------------|
| `mapping_t::thread_t` | `mapping.thread` | Execution agents are mapped onto threads of execution. |
| `mapping_t::new_thread_t` | `mapping.new_thread` | Each execution agent is mapped onto a new thread of execution. |
| `mapping_t::other_t` | `mapping.other` | Mapping of each execution agent is implementation-defined. |

[*Note:* A mapping of an execution agent onto a thread of execution implies the execution
agent runs as-if on a `std::thread`. Therefore, the facilities provided by
`std::thread`, such as thread-local storage, are available.
`mapping_t::new_thread_t` provides stronger guarantees, in
particular that thread-local storage will not be shared between execution
agents. *--end note*]

#### Properties for handling exceptions thrown by one-way submitted functions

The `oneway_exception_handler_t` property describes how exceptions thrown by one-way submitted functions are handled.

`oneway_exception_handler_t` provides nested property types and objects as indicated below.

| Nested Property Type| Nested Property Object Name | Requirements |
|-------------------------|---------------------------------|--------------|
| `oneway_exception_handler_t::other_t` | `oneway_exception_handler.other` | The behavior of a function invoked by an execution agent created by `execute` or `bulk_execute` that exits via an uncaught exception is implementation-defined.  |
| `oneway_exception_handler_t::terminate_t` | `oneway_exception_handler.terminate` | If a function invoked by an execution agent created by `execute` or `bulk_execute` exits via an uncaught exception, `terminate()` shall be called. |

### Properties for customizing memory allocation

	template <typename ProtoAllocator>
	struct allocator_t;

The `allocator_t` property conforms to the following specification:

    template <typename ProtoAllocator>
    struct allocator_t
    {
        template <class T>
          static constexpr bool is_applicable_property_v = is_executor_v<T>;

        static constexpr bool is_requirable = true;
        static constexpr bool is_preferable = true;
    
        template<class Executor>
        static constexpr auto static_query_v
          = Executor::query(allocator_t);
    
        template <typename OtherProtoAllocator>
        allocator_t<OtherProtoAllocator> operator()(const OtherProtoAllocator &a) const;
    
        static constexpr ProtoAllocator value() const;
    
    private:
        ProtoAllocator a_; // exposition only
    };

| Property | Notes | Requirements |
|----------|-------|--------------|
| `allocator_t<ProtoAllocator>` | Result of `allocator_t<void>::operator(OtherProtoAllocator)`. | The executor shall use the encapsulated allocator to allocate any memory required to store the submitted function object. |
| `allocator_t<void>` | Specialisation of `allocator_t<ProtoAllocator>`. | The executor shall use an implementation defined default allocator to allocate any memory required to store the submitted function object. |

If the expression `execution::query(E, P)` is well formed, where `P` is an object of type `allocator_t<ProtoAllocator>`, then:
* the type of the expression `execution::query(E, P)` shall satisfy the `ProtoAllocator` requirements;
* the result of the expression `execution::query(E, P)` shall be the allocator currently established in the executor `E`; and
* the expression `execution::query(E, allocator_t<void>{})` shall also be well formed and have the same result as `execution::query(E, P)`.

#### `allocator_t` members

```
template <typename OtherProtoAllocator>
allocator_t<OtherProtoAllocator> operator()(const OtherProtoAllocator &a) const;
```

*Returns:* An allocator object whose exposition-only member `a_` is initialized as `a_(a)`.

*Remarks:* This function shall not participate in overload resolution unless `ProtoAllocator` is `void`.

[*Note:* It is permitted for `a` to be an executor's implementation-defined default allocator and, if so, the default allocator may also be established within an executor by passing the result of this function to `require`. *--end note*]

```
static constexpr ProtoAllocator value() const;
```

*Returns:* The exposition-only member `a_`.

*Remarks:* This function shall not participate in overload resolution unless `ProtoAllocator` is not `void`.

## Executor type traits

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

## Polymorphic executor support

### Class `bad_executor`

An exception of type `bad_executor` is thrown by polymorphic executor member functions `execute` and `bulk_execute` when the executor object has no target.

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

### Struct `prefer_only`

The `prefer_only` struct is a property adapter that disables the `is_requirable` value.

[*Example:*

Consider a generic function that performs some task immediately if it can, and otherwise asynchronously in the background.

    template<class Executor, class Callback>
    void do_async_work(
        Executor ex,
        Callback callback)
    {
      if (try_work() == done)
      {
        // Work completed immediately, invoke callback.
        execution::require(ex,
            execution::single,
            execution::oneway,
          ).execute(callback);
      }
      else
      {
        // Perform work in background. Track outstanding work.
        start_background_work(
            execution::prefer(ex,
              execution::outstanding_work.tracked),
            callback);
      }
    }

This function can be used with an inline executor which is defined as follows:

    struct inline_executor
    {
      constexpr bool operator==(const inline_executor&) const noexcept
      {
        return true;
      }
    
      constexpr bool operator!=(const inline_executor&) const noexcept
      {
        return false;
      }
    
      template<class Function> void execute(Function f) const noexcept
      {
        f();
      }
    };

as, in the case of an unsupported property, invocation of `execution::prefer` will fall back to an identity operation.

The polymorphic `executor` wrapper should be able to simply swap in, so that we could change `do_async_work` to the non-template function:

    void do_async_work(
        executor<
          execution::single,
          execution::oneway,
          execution::outstanding_work_t::tracked_t> ex,
        std::function<void()> callback)
    {
      if (try_work() == done)
      {
        // Work completed immediately, invoke callback.
        execution::require(ex,
            execution::single,
            execution::oneway,
          ).execute(callback);
      }
      else
      {
        // Perform work in background. Track outstanding work.
        start_background_work(
            execution::prefer(ex,
              execution::outstanding_work.tracked),
            callback);
      }
    }

with no change in behavior or semantics.

However, if we simply specify `execution::outstanding_work.tracked` in the `executor` template parameter list, we will get a compile error due to the `executor` template not knowing that `execution::outstanding_work.tracked` is intended for use with `prefer` only. At the point of construction from an `inline_executor` called `ex`, `executor` will try to instantiate implementation templates that perform the ill-formed `execution::require(ex, execution::outstanding_work.tracked)`.

The `prefer_only` adapter addresses this by turning off the `is_requirable` attribute for a specific property. It would be used in the above example as follows:

    void do_async_work(
        executor<
          execution::single,
          execution::oneway,
          prefer_only<execution::outstanding_work_t::tracked_t>> ex,
        std::function<void()> callback)
    {
      ...
    }

*-- end example*]

    template<class InnerProperty>
    struct prefer_only
    {
      InnerProperty property;
    
      static constexpr bool is_requirable = false;
      static constexpr bool is_preferable = InnerProperty::is_preferable;
    
      using polymorphic_query_result_type = see-below; // not always defined
    
      template<class Executor>
        static constexpr auto static_query_v = see-below; // not always defined
    
      constexpr prefer_only(const InnerProperty& p);
    
      constexpr auto value() const
        noexcept(noexcept(std::declval<const InnerProperty>().value()))
          -> decltype(std::declval<const InnerProperty>().value());
    
      template<class Executor, class Property>
      friend auto prefer(Executor ex, const Property& p)
        noexcept(noexcept(execution::prefer(std::move(ex), std::declval<const InnerProperty>())))
          -> decltype(execution::prefer(std::move(ex), std::declval<const InnerProperty>()));
    
      template<class Executor, class Property>
      friend constexpr auto query(const Executor& ex, const Property& p)
        noexcept(noexcept(execution::query(ex, std::declval<const InnerProperty>())))
          -> decltype(execution::query(ex, std::declval<const InnerProperty>()));
    };

If `InnerProperty::polymorphic_query_result_type` is valid and denotes a type, the template instantiation `prefer_only<InnerProperty>` defines a nested type `polymorphic_query_result_type` as a synonym for `InnerProperty::polymorphic_query_result_type`.

If `InnerProperty::static_query_v` is a variable template and `InnerProperty::static_query_v<E>` is well formed for some executor type `E`, the template instantiation `prefer_only<InnerProperty>` defines a nested variable template `static_query_v` as a synonym for `InnerProperty::static_query_v`.

```
constexpr prefer_only(const InnerProperty& p);
```

*Effects:* Initializes `property` with `p`.

```
constexpr auto value() const
  noexcept(noexcept(std::declval<const InnerProperty>().value()))
    -> decltype(std::declval<const InnerProperty>().value());
```

*Returns:* `property.value()`.

*Remarks:* Shall not participate in overload resolution unless the expression `property.value()` is well-formed.

```
template<class Executor, class Property>
friend auto prefer(Executor ex, const Property& p)
  noexcept(noexcept(execution::prefer(std::move(ex), std::declval<const InnerProperty>())))
    -> decltype(execution::prefer(std::move(ex), std::declval<const InnerProperty>()));
```

*Returns:* `execution::prefer(std::move(ex), p.property)`.

*Remarks:* Shall not participate in overload resolution unless `std::is_same_v<Property, prefer_only>` is `true`, and the expression `execution::prefer(std::move(ex), p.property)` is well-formed.

```
template<class Executor, class Property>
friend constexpr auto query(const Executor& ex, const Property& p)
  noexcept(noexcept(execution::query(ex, std::declval<const InnerProperty>())))
    -> decltype(execution::query(ex, std::declval<const InnerProperty>()));
```

*Returns:* `execution::query(ex, p.property)`.

*Remarks:* Shall not participate in overload resolution unless `std::is_same_v<Property, prefer_only>` is `true`, and the expression `execution::query(ex, p.property)` is well-formed.

## Thread pools

Thread pools manage execution agents which run on threads without incurring the
overhead of thread creation and destruction whenever such agents are needed.

### Header `<thread_pool>` synopsis

```
namespace std {

  class static_thread_pool;

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

[*Note:* Because `static_thread_pool` represents work as parallel execution agents,
situations which require concurrent execution properties are not guaranteed
correctness. *--end note.*]

```
class static_thread_pool
{
  public:
    using scheduler_type = see-below;
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

    // placeholder for a general approach to getting schedulers from 
    // standard contexts.
    scheduler_type scheduler() noexcept;

    // placeholder for a general approach to getting executors from 
    // standard contexts.
    executor_type executor() noexcept;
};
```

For an object of type `static_thread_pool`, *outstanding work* is defined as the sum
of:

* the number of existing executor objects associated with the
  `static_thread_pool` for which the `execution::outstanding_work.tracked` property is
  established;

* the number of function objects that have been added to the `static_thread_pool`
  via the `static_thread_pool` executor, scheduler and sender, but not yet invoked; and

* the number of function objects that are currently being invoked within the
  `static_thread_pool`.

The `static_thread_pool` member functions `scheduler`, `executor`, `attach`, 
`wait`, and `stop`, and the associated schedulers', senders` and executors' copy 
constructors and member functions, do not introduce data races as a result of 
concurrent invocations of those functions from different threads of execution.

A `static_thread_pool`'s threads run execution agents with forward progress guarantee delegation. [*Note:* Forward progress is delegated to an execution agent for its lifetime. Because `static_thread_pool` guarantees only parallel forward progress to running execution agents; _i.e._, execution agents which have run the first step of the function object. *--end note*]

#### Types

```
using scheduler_type = see-below;
```

A scheduler type conforming to the specification for `static_thread_pool` scheduler types described below.

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
after completion of that function object. Invocation of `stop()` returns without
waiting for the threads to complete. Subsequent invocations to attach complete
immediately.

```
void wait();
```

*Effects:* If not already stopped, signals the threads in the pool to complete
once the outstanding work is `0`. Blocks the calling thread (C++Std
[defns.block]) until all threads in the pool have completed, without executing
submitted function objects in the calling thread. Subsequent invocations of `attach()`
complete immediately.

*Synchronization:* The completion of each thread in the pool synchronizes with
(C++Std [intro.multithread]) the corresponding successful `wait()` return.

#### Scheduler creation

```
scheduler_type scheduler() noexcept;
```

*Returns:* A scheduler that may be used to create sender objects that may be 
used to submit receiver objects to the thread pool. The returned scheduler has 
the following properties already established:

  * `execution::allocator`
  * `execution::allocator(std::allocator<void>())`

#### Executor creation

```
executor_type executor() noexcept;
```

*Returns:* An executor that may be used to submit function objects to the
thread pool. The returned executor has the following properties already
established:

  * `execution::oneway`
  * `execution::blocking.possibly`
  * `execution::relationship.fork`
  * `execution::outstanding_work.untracked`
  * `execution::allocator`
  * `execution::allocator(std::allocator<void>())`

### `static_thread_pool` scheduler types

All scheduler types accessible through `static_thread_pool::scheduler()`, and subsequent invocations of the member function `require`, conform to the following specification.

```
class C
{
  public:

    // types:

    using sender_type = see-below;

    // construct / copy / destroy:

    C(const C& other) noexcept;
    C(C&& other) noexcept;

    C& operator=(const C& other) noexcept;
    C& operator=(C&& other) noexcept;

    // scheduler operations:

    see-below require(const execution::allocator_t<void>& a) const;
    template<class ProtoAllocator>
    see-below require(const execution::allocator_t<ProtoAllocator>& a) const;

    see-below query(execution::context_t) const noexcept;
    see-below query(execution::allocator_t<void>) const noexcept;
    template<class ProtoAllocator>
    see-below query(execution::allocator_t<ProtoAllocator>) const noexcept;

    bool running_in_this_thread() const noexcept;
};

bool operator==(const C& a, const C& b) noexcept;
bool operator!=(const C& a, const C& b) noexcept;
```

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
see-below require(const execution::allocator_t<void>& a) const;
```

*Returns:* `require(execution::allocator(x))`, where `x` is an implementation-defined default allocator.

```
template<class ProtoAllocator>
  see-below require(const execution::allocator_t<ProtoAllocator>& a) const;
```

*Returns:* An scheduler object of an unspecified type conforming to these
specifications, associated with the same thread pool as `*this`, with the
`execution::allocator_t<ProtoAllocator>` property established such that
allocation and deallocation associated with function submission will be
performed using a copy of `a.alloc`. All other properties of the returned
scheduler object are identical to those of `*this`.

```
static_thread_pool& query(execution::context_t) const noexcept;
```

*Returns:* A reference to the associated `static_thread_pool` object.

```
see-below query(execution::allocator_t<void>) const noexcept;
see-below query(execution::allocator_t<ProtoAllocator>) const noexcept;
```

*Returns:* The allocator object associated with the executor, with type and
value as either previously established by the `execution::allocator_t<ProtoAllocator>`
property or the implementation defined default allocator established by the `execution::allocator_t<void>` property.

```
bool running_in_this_thread() const noexcept;
```

*Returns:* `true` if the current thread of execution is a thread that was
created by or attached to the associated `static_thread_pool` object.

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

#### `static_thread_pool` scheduler functions

In addition to conforming to the above specification, `static_thread_pool`
schedulers shall conform to the following specification.

```
class C
{
  public:
    sender_type schedule() noexcept;
};
```

`C` is a type satisfying the `Scheduler` requirements.

#### Sender creation

```
  sender_type schedule() noexcept;
```

*Returns:* A sender that may be used to submit function objects to the
thread pool. The returned sender has the following properties already
established:

  * `execution::oneway`
  * `execution::blocking.possibly`
  * `execution::relationship.fork`
  * `execution::outstanding_work.untracked`
  * `execution::allocator`
  * `execution::allocator(std::allocator<void>())`

### `static_thread_pool` sender types

All sender types accessible through `static_thread_pool::scheduler().schedule()`, and subsequent invocations of the member function `require`, conform to the following specification.

```
class C
{
  public:

    // construct / copy / destroy:

    C(const C& other) noexcept;
    C(C&& other) noexcept;

    C& operator=(const C& other) noexcept;
    C& operator=(C&& other) noexcept;

    // sender operations:

    see-below require(execution::blocking_t::never_t) const;
    see-below require(execution::blocking_t::possibly_t) const;
    see-below require(execution::blocking_t::always_t) const;
    see-below require(execution::relationship_t::continuation_t) const;
    see-below require(execution::relationship_t::fork_t) const;
    see-below require(execution::outstanding_work_t::tracked_t) const;
    see-below require(execution::outstanding_work_t::untracked_t) const;
    see-below require(const execution::allocator_t<void>& a) const;
    template<class ProtoAllocator>
    see-below require(const execution::allocator_t<ProtoAllocator>& a) const;

    static constexpr execution::bulk_guarantee_t query(execution::bulk_guarantee_t::parallel_t) const;
    static constexpr execution::mapping_t query(execution::mapping_t::thread_t) const;
    execution::blocking_t query(execution::blocking_t) const;
    execution::relationship_t query(execution::relationship_t) const;
    execution::outstanding_work_t query(execution::outstanding_work_t) const;
    see-below query(execution::context_t) const noexcept;
    see-below query(execution::allocator_t<void>) const noexcept;
    template<class ProtoAllocator>
    see-below query(execution::allocator_t<ProtoAllocator>) const noexcept;

    bool running_in_this_thread() const noexcept;
};

bool operator==(const C& a, const C& b) noexcept;
bool operator!=(const C& a, const C& b) noexcept;
```

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
see-below require(execution::blocking_t::never_t) const;
see-below require(execution::blocking_t::possibly_t) const;
see-below require(execution::blocking_t::always_t) const;
see-below require(execution::relationship_t::continuation_t) const;
see-below require(execution::relationship_t::fork_t) const;
see-below require(execution::outstanding_work_t::tracked_t) const;
see-below require(execution::outstanding_work_t::untracked_t) const;
```

*Returns:* An sender object of an unspecified type conforming to these
specifications, associated with the same thread pool as `*this`, and having the
requested property established. When the requested property is part of a group
that is defined as a mutually exclusive set, any other properties in the group
are removed from the returned sender object. All other properties of the
returned sender object are identical to those of `*this`.

```
see-below require(const execution::allocator_t<void>& a) const;
```

*Returns:* `require(execution::allocator(x))`, where `x` is an implementation-defined default allocator.

```
template<class ProtoAllocator>
  see-below require(const execution::allocator_t<ProtoAllocator>& a) const;
```

*Returns:* An sender object of an unspecified type conforming to these
specifications, associated with the same thread pool as `*this`, with the
`execution::allocator_t<ProtoAllocator>` property established such that
allocation and deallocation associated with function submission will be
performed using a copy of `a.alloc`. All other properties of the returned
sender object are identical to those of `*this`.

```
static constexpr execution::bulk_guarantee_t query(execution::bulk_guarantee_t) const;
```

*Returns:* `execution::bulk_guarantee.parallel`

```
static constexpr execution::mapping_t query(execution::mapping_t) const;
```

*Returns:* `execution::mapping.thread`.

```
execution::blocking_t query(execution::blocking_t) const;
execution::relationship_t query(execution::relationship_t) const;
execution::outstanding_work_t query(execution::outstanding_work_t) const;
```

*Returns:* The value of the given property of `*this`.

```
static_thread_pool& query(execution::context_t) const noexcept;
```

*Returns:* A reference to the associated `static_thread_pool` object.

```
see-below query(execution::allocator_t<void>) const noexcept;
see-below query(execution::allocator_t<ProtoAllocator>) const noexcept;
```

*Returns:* The allocator object associated with the sender, with type and
value as either previously established by the `execution::allocator_t<ProtoAllocator>`
property or the implementation defined default allocator established by the `execution::allocator_t<void>` property.

```
bool running_in_this_thread() const noexcept;
```

*Returns:* `true` if the current thread of execution is a thread that was
created by or attached to the associated `static_thread_pool` object.

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

#### `static_thread_pool` sender execution functions

In addition to conforming to the above specification, `static_thread_pool`
executors shall conform to the following specification.

```
class C
{
  public:
    template<class Receiver>
      void submit(Receiver&& r) const;
};
```

`C` is a type satisfying the `Sender` requirements.

```
template<class Receiver>
  void submit(Receiver&& r) const;
```

*Effects:* Submits the receiver `r` for execution on the `static_thread_pool`
according to the the properties established for `*this`. let `e` be an object of type `exception_ptr`, then `static_thread_pool` will evaluate one of `set_value(r)`, `set_error(r, e)`, or `set_done(r)`.

### `static_thread_pool` executor types

All executor types accessible through `static_thread_pool::executor()`, and subsequent invocations of the member function `require`, conform to the following specification.

```
class C
{
  public:

    // indication of applicability to executor properties
    static constexpr bool is_executor_v = true;

    // types:

    using shape_type = size_t;
    using index_type = size_t;

    // construct / copy / destroy:

    C(const C& other) noexcept;
    C(C&& other) noexcept;

    C& operator=(const C& other) noexcept;
    C& operator=(C&& other) noexcept;

    // executor operations:

    see-below require(execution::blocking_t::never_t) const;
    see-below require(execution::blocking_t::possibly_t) const;
    see-below require(execution::blocking_t::always_t) const;
    see-below require(execution::relationship_t::continuation_t) const;
    see-below require(execution::relationship_t::fork_t) const;
    see-below require(execution::outstanding_work_t::tracked_t) const;
    see-below require(execution::outstanding_work_t::untracked_t) const;
    see-below require(const execution::allocator_t<void>& a) const;
    template<class ProtoAllocator>
    see-below require(const execution::allocator_t<ProtoAllocator>& a) const;

    static constexpr execution::bulk_guarantee_t query(execution::bulk_guarantee_t::parallel_t) const;
    static constexpr execution::mapping_t query(execution::mapping_t::thread_t) const;
    execution::blocking_t query(execution::blocking_t) const;
    execution::relationship_t query(execution::relationship_t) const;
    execution::outstanding_work_t query(execution::outstanding_work_t) const;
    see-below query(execution::context_t) const noexcept;
    see-below query(execution::allocator_t<void>) const noexcept;
    template<class ProtoAllocator>
    see-below query(execution::allocator_t<ProtoAllocator>) const noexcept;

    bool running_in_this_thread() const noexcept;
};

bool operator==(const C& a, const C& b) noexcept;
bool operator!=(const C& a, const C& b) noexcept;
```

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
see-below require(execution::blocking_t::never_t) const;
see-below require(execution::blocking_t::possibly_t) const;
see-below require(execution::blocking_t::always_t) const;
see-below require(execution::relationship_t::continuation_t) const;
see-below require(execution::relationship_t::fork_t) const;
see-below require(execution::outstanding_work_t::tracked_t) const;
see-below require(execution::outstanding_work_t::untracked_t) const;
```

*Returns:* An executor object of an unspecified type conforming to these
specifications, associated with the same thread pool as `*this`, and having the
requested property established. When the requested property is part of a group
that is defined as a mutually exclusive set, any other properties in the group
are removed from the returned executor object. All other properties of the
returned executor object are identical to those of `*this`.

```
see-below require(const execution::allocator_t<void>& a) const;
```

*Returns:* `require(execution::allocator(x))`, where `x` is an implementation-defined default allocator.

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
static constexpr execution::bulk_guarantee_t query(execution::bulk_guarantee_t) const;
```

*Returns:* `execution::bulk_guarantee.parallel`

```
static constexpr execution::mapping_t query(execution::mapping_t) const;
```

*Returns:* `execution::mapping.thread`.

```
execution::blocking_t query(execution::blocking_t) const;
execution::relationship_t query(execution::relationship_t) const;
execution::outstanding_work_t query(execution::outstanding_work_t) const;
```

*Returns:* The value of the given property of `*this`.

```
static_thread_pool& query(execution::context_t) const noexcept;
```

*Returns:* A reference to the associated `static_thread_pool` object.

```
see-below query(execution::allocator_t<void>) const noexcept;
see-below query(execution::allocator_t<ProtoAllocator>) const noexcept;
```

*Returns:* The allocator object associated with the executor, with type and
value as either previously established by the `execution::allocator_t<ProtoAllocator>`
property or the implementation defined default allocator established by the `execution::allocator_t<void>` property.

```
bool running_in_this_thread() const noexcept;
```

*Returns:* `true` if the current thread of execution is a thread that was
created by or attached to the associated `static_thread_pool` object.

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

#### `static_thread_pool` executor execution functions

In addition to conforming to the above specification, `static_thread_pool`
executors shall conform to the following specification.

```
class C
{
  public:
    template<class Function>
      void execute(Function&& f) const;

    template<class Function>
      void bulk_execute(Function&& f, size_t n) const;
};
```

`C` is a type satisfying the `Executor` requirements.

```
template<class Function>
  void execute(Function&& f) const;
```

*Effects:* Submits the function `f` for execution on the `static_thread_pool`
according to the the properties established for `*this`. If the submitted
function `f` exits via an exception, the `static_thread_pool` invokes
`std::terminate()`.

```
template<class Function>
  void bulk_execute(Function&& f, size_t n) const;
```

*Effects:* Submits the function `f` for bulk execution on the
`static_thread_pool` according to properties established for `*this`. If the
submitted function `f` exits via an exception, the `static_thread_pool` invokes
`std::terminate()`.

