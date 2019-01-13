
----------------    -------------------------------------
Title:              A General Property Customization Mechanism 

Authors:            David Hollman, dshollm@sandia.gov

                    Chris Kohlhoff, chris@kohlhoff.com

                    Bryce Lelbach, brycelelbach@gmail.com

                    Jared Hoberock, jhoberock@nvidia.com

                    Gordon Brown, gordon@codeplay.com

Other Contributors: Lee Howes, lwh@fb.com

                    Michael Garland, mgarland@nvidia.com

                    Chris Mysen, mysen@google.com

                    Thomas Rodgers, rodgert@twrodgers.com

                    Michael Wong, michael@codeplay.com

Document Number:    D1393R0

Date:               2019-01-13

Audience:           LEWG

Reply-to:           sg1-exec@googlegroups.com

Abstract:           This paper generalizes and extracts the property customization mechanism from [P0443r8](http://wg21.link/p0443r8), as requested by LEWG in the 2018-11 San Diego meeting.  This document does not introduce any significant design changes from the design previously discussed in the context of P0443; the separation herein is merely a recognition of the mechanism's general applicability and is made in anticipation of its use in other contexts.

------------------------------------------------------

## Changelog

### Revision 0

* Initial design

# Introduction

TODO: Write something here.

# Proposed Wording

Add the following row to the table in **[utilities.general]**:


--------------  ---------------------------------------------------   --------------
                **Subclause**                                         **Header(s)** 

[properties]    Support for the property customization mechanism      `<property>`

--------------  ---------------------------------------------------   --------------


Add the following subclause to **[utilities]** in a section which the editor shall determine:

## Properties Support

### General

This subclause describes components supporting an extensible customization mechanism, currently used most prominently by the execution support library **[execution]**.

TODO: more here?

### Header `<property>` synopsis

```
namespace std {

  // Customization point objects:

  inline namespace unspecified {
    inline constexpr unspecified require_concept = unspecified;
    inline constexpr unspecified require = unspecified;
    inline constexpr unspecified prefer = unspecified;
    inline constexpr unspecified query = unspecified;
  }

  // Customization point type traits:

  template<class T, class P> struct can_require_concept;
  template<class T, class P> struct can_require;
  template<class T, class P> struct can_prefer;
  template<class T, class P> struct can_query;
  template<class T, class P> struct is_applicable_property;

  template<class T, class... Properties>
    inline constexpr bool can_require_concept_v = can_require_concept<T, Properties...>::value;
  template<class T, class... Properties>
    inline constexpr bool can_require_v = can_require<T, Properties...>::value;
  template<class T, class... Properties>
    inline constexpr bool can_prefer_v = can_prefer<T, Properties...>::value;
  template<class T, class Property>
    inline constexpr bool can_query_v = can_query<T, Property>::value;
  template<class T, class Property>
    inline constexpr bool is_applicable_property_v = is_applicable_property<T, Property>::value;

} // namespace std
```

## Customization point objects

<!-- Mimicking customization point object specification in [range.access] -->

<!-- TODO figure out if we want to use something like DECAY_COPY for copyable types? -->

### `require_concept`

```
inline namespace unspecified {
  inline constexpr unspecified require_concept = unspecified;
}
```

The name `require_concept` denotes a customization point object. The expression `std::require_concept(E, P)` for some subexpressions `E` and `P` (with types `T = decay_t<decltype(E)>` and `Prop = decay_t<decltype(P)>`) is expression-equivalent to:

* If `is_applicable_property_v<T, Prop> && Prop::is_requirable_concept` is not a well-formed constant expression with value `true`, `std::require_concept(E, P)` is ill-formed.

* Otherwise, `E` if the expression `Prop::template static_query_v<T> == Prop::value()` is a well-formed constant expression with value `true`.

* Otherwise, `(E).require_concept(P)` if the expression `(E).require_concept(P)` is well-formed.

* Otherwise, `require_concept(E, P)` if the expression `require_concept(E, P)` is a valid expression with overload resolution performed in a context that does not include the declaration of the `require_concept` customization point object.

* Otherwise, `std::require_concept(E, P)` is ill-formed.  

### `require`

```
inline namespace unspecified {
  inline constexpr unspecified require = unspecified;
}
```

The name `require` denotes a customization point object. The expression `std::require(E, P0, Pn...)` for some subexpressions `E` and `P0`, and where `Pn...` represents `N` subexpressions (where `N` is 0 or more, and with types `T = decay_t<decltype(E)>` and `Prop0 = decay_t<decltype(P0)>`) is expression-equivalent to:

* If `is_applicable_property_v<T, Prop0> && Prop0::is_requirable` is not a well-formed constant expression with value `true`, `std::require(E, P0, Pn...)` is ill-formed.

* Otherwise, `E` if `N == 0` and the expression `Prop0::template static_query_v<T> == Prop0::value()` is a well-formed constant expression with value `true`.

* Otherwise, `(E).require(P0)` if `N == 0` and the expression `(E).require(P0)` is a valid expression.

* Otherwise, `require(E, P)` if `N == 0` and the expression `require(E, P)` is a valid expression with overload resolution performed in a context that does not include the declaration of the `require` customization point object.

* Otherwise, `std::require(std::require(E, P0), Pn...)` if `N > 0` and the expression `std::require(std::require(E, P0), Pn...)` is a valid expression.

* Otherwise, `std::require(E, P0, Pn...)` is ill-formed.  

### `prefer`

```
inline namespace unspecified {
  inline constexpr unspecified prefer = unspecified;
}
```

The name `prefer` denotes a customization point object. The expression `std::prefer(E, P0, Pn...)` for some subexpressions `E` and `P0`, and where `Pn...` represents `N` subexpressions (where `N` is 0 or more, and with types `T = decay_t<decltype(E)>` and `Prop0 = decay_t<decltype(P0)>`) is expression-equivalent to:

* If `is_applicable_property_v<T, Prop0> && Prop0::is_preferable` is not a well-formed constant expression with value `true`, `std::prefer(E, P0, Pn...)` is ill-formed.

* Otherwise, `E` if `N == 0` and the expression `Prop0::template static_query_v<T> == Prop0::value()` is a well-formed constant expression with value `true`.

* Otherwise, `(E).require(P0)` if `N == 0` and the expression `(E).require(P0)` is a valid expression.

* Otherwise, `require(E, P0)` if `N == 0` and the expression `require(E, P0)` is a valid expression with overload resolution performed in a context that does not include the declaration of the `require` customization point object.

* Otherwise, `std::prefer(std::prefer(E, P0), Pn...)` if `N > 0` and the expression `std::prefer(std::prefer(E, P0), Pn...)` is a valid expression.

* Otherwise, `std::prefer(E, P0, Pn...)` is ill-formed.

### `query`

```
inline namespace unspecified {
  inline constexpr unspecified query = unspecified;
}
```

The name `query` denotes a customization point object. The expression `std::query(E, P)` for some subexpressions `E` and `P` (with types `T = decay_t<decltype(E)>` and `Prop = decay_t<decltype(P)>`) is expression-equivalent to:

* If `is_applicable_property_v<T, Prop>` is not a well-formed constant expression with value `true`, `std::query(E, P)` is ill-formed.

* Otherwise, `Prop::template static_query_v<T>` if the expression `Prop::template static_query_v<T>` is a well-formed constant expression.

* Otherwise, `(E).query(P)` if the expression `(E).query(P)` is well-formed.

* Otherwise, `query(E, P)` if the expression `query(E, P)` is a valid expression with overload resolution performed in a context that does not include the declaration of the `query` customization point object.

* Otherwise, `std::query(E, P)` is ill-formed.

## Customization point type traits

```
template<class Executor, class... Properties> struct can_require;
template<class Executor, class... Properties> struct can_prefer;
template<class Executor, class Property> struct can_query;
```

This sub-clause contains templates that may be used to query the validity of the application of property customization point objects to a type at compile time. Each of these templates is a UnaryTypeTrait (C++Std [meta.rqmts]) with a BaseCharacteristic of `true_type` if the corresponding condition is true, otherwise `false_type`.

| Template                   | Condition           | Preconditions  |
|----------------------------|---------------------|----------------|
| `template<class T, class P>` <br/>`struct can_require_concept` | The expression `std::require_concept(declval<const T>(), declval<P>())` is well-formed. | `T` and `P` are complete types. |
| `template<class T, class P>` <br/>`struct can_require` | The expression `std::require(declval<const T>(), declval<P>())` is well-formed. | `T` and `P` are complete types. |
| `template<class T, class P>` <br/>`struct can_prefer` | The expression `std::prefer(declval<const T>(), declval<P>())` is well-formed. | `T` and `P` are complete types. |
| `template<class T, class P>` <br/>`struct can_query` | The expression `std::query(declval<const T>(), declval<P>())` is well-formed. | `T` and `P` are complete types. |
| `template<class T, class P>` <br/>`struct is_applicable_property` | The expression `P::template is_applicable_property_v<T>` is a well-formed constant expression with a value of `true`. | `P` and `T` are complete types. |

## The property customization mechanism

### In general

When the property customization mechanism is being employed for some library facility, an object's behavior and effects on that facility in generic contexts may be determined by a set of applicable properties, and each property imposes certain requirements on that object's behavior or exposes some attribute of that object.

<!--
[*Note:* As a general design note properties which define a mutually exclusive pair, that describe an enabled or non-enabled behaviour follow the convention of having the same property name for both with the `not_` prefix to the property for the non-enabled behaviour. *--end note*]


Given an existing executor, a related executor with different properties may be created by invoking the `require` member or non-member functions. These functions behave according the Table below. In the Table below, `x` denotes a (possibly const) executor object of type `X`, and `p` denotes a (possibly const) property object.


| Expression | Comments |
|------------|----------|
| `x.require(p)` <br/> `require(x,p)` | Returns an executor object with the requested property `p` added to the set. All other properties of the returned executor are identical to those of `x`, except where those properties are described below as being mutually exclusive to `p`. In this case, the mutually exclusive properties are implicitly removed from the set associated with the returned executor. <br/> <br/> The expression is ill formed if an executor is unable to add the requested property. |

The current value of an executor's properties can be queried by invoking the `query` function. This function behaves according the Table below. In the Table below, `x` denotes a (possibly const) executor object of type `X`, and `p` denotes a (possibly const) property object.

| Expression | Comments |
|------------|----------|
| `x.query(p)` | Returns the current value of the requested property `p`. The expression is ill formed if an executor is unable to return the requested property. |
-->

### Requirements on properties

A property type `P` shall provide:

* A nested constant variable template named `is_applicable_property_v`, usable as `P::template is_applicable_property_v<T>` on any complete type `T`, with a value of type `bool`.

A property type `P` that is not an interface-changing shall provide:

* A nested constant expression named `is_requirable` of type `bool`, usable as `P::is_requirable`.
* A nested constant expression named `is_preferable` of type `bool`, usable as `P::is_preferable`.

A property type `P` that is interface-changing shall provide:

* A nested constant expression named `is_requirable_concept` of type `bool`, usable as `P::is_requirable_concept`.

A property type `P` may provide a nested type `polymorphic_query_result_type` that satisfies the `CopyConstructible` and `Destructible` requirements. If `P::is_requirable == true` or `P::is_preferable == true`, `polymorphic_query_result_type` shall also satisfy the `DefaultConstructible` requirements. [*Note:* When present, this type allows the property to be used with polymorphic wrappers. *--end note*]

A property type `P` may provide:

* A nested variable template `static_query_v`, usable as `P::template static_query_v<T>`. This may be conditionally present.
* A member function `value()`.

If both `static_query_v` and `value()` are present, they shall return the same type and this type shall satisfy the `EqualityComparable` requirements.

[*Note:* These are used to determine whether invoking `require` or `require_concept` would result in an identity transformation. *—end note*]

A property type `P` that is interface-changing may provide:

* A nested template or named `polymorphic_wrapper_type`, usable with properties `Ps...` that are not interface-changing as `typename P::template polymorphic_wrapper_type<Ps...>`.  The type `PW` resulting from the instantiation of this template shall:
    * be implicitly constructible from a (potentially `const`) instance `t` of a type `T`, where:
        * `P::template static_query_v<T> == P::value()`,
        * `(can_query_v<Ps, T> ... && ... true)` is `true`
        * `can_require_v<Pn, T>` is `true` for all `Pn` in `Ps...` with `Pn::is_requirable == true`
        * `can_prefer_v<Pn, T>` is `true` for all `Pn` in `Ps...` with `Pn::is_preferable == true`
    * be valid in the constant expression `is_applicable_property_v<P, PW>`, and that expression shall be `true`.
    * if both `static_query_v` and `value()` are present in `P`, then `PW` shall be valid in the constant expression `static_query_v<P, PW> == P::value()`, and that expression shall be `true`.


[*Note:* 
For example, the struct `S` provides the type for a property that is interface-changing:

```
struct S
{
  static constexpr bool is_requirable_concept = true;

  template<class... Ps> 
    class polymorphic_wrapper_type;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool static_query_v
      = see-below;

  static constexpr bool value() const { return true; }
};
```

*—end note*]

#### Behavioral properties

Behavioral properties define a set of mutually-exclusive nested properties describing executor behavior.

Behavioral property types `S` applicable to objects meeting the requirements of some concept `C`, their nested property types `S::N`*i*, and nested property objects `S::n`*i* conform to the following specification:

```
struct S
{

  template <class T>
    static constexpr bool is_applicable_property_v = C<T>;

  static constexpr bool is_requirable = false;
  static constexpr bool is_preferable = false;
  using polymorphic_query_result_type = S;

  template<class T>
    static constexpr auto static_query_v
      = see-below;

  template<class T>
  friend constexpr S query(const T& ex, const Property& p) noexcept(see-below);

  friend constexpr bool operator==(const S& a, const S& b);
  friend constexpr bool operator!=(const S& a, const S& b) { return !operator==(a, b); }

  constexpr S();

  struct N1
  {
    template <class T>
      static constexpr bool is_applicable_property_v = C<T>;

    static constexpr bool is_requirable = true;
    static constexpr bool is_preferable = true;
    using polymorphic_query_result_type = S;

    template<class T>
      static constexpr auto static_query_v
        = see-below;

    static constexpr S value() { return S(N1()); }
  };

  static constexpr n1;

  constexpr S(const N1);

  /* ... */

  struct NN
  {
    template <class T>
      static constexpr bool is_applicable_property_v = C<T>;

    static constexpr bool is_requirable = true;
    static constexpr bool is_preferable = true;
    using polymorphic_query_result_type = S;

    template<class T>
      static constexpr auto static_query_v
        = see-below;

    static constexpr S value() { return S(NN()); }
  };

  static constexpr nN;

  constexpr S(const NN);
};
```

Behavioral properties shall not be interface-changing.

Queries for the value of an object's behavioral property shall not change between invocations unless the object is assigned another object with a different value of that behavioral property.

`S()` and `S(S::E`*i*`())` are all distinct values of `S`. [*Note:* This means they compare unequal. *--end note.*]

The value returned from `std::query(e1, p1)` and a subsequent invocation `std::query(e2, p1)`, where

* `p1` is an instance of `S` or `S::E`*i*, and
* `e2` is the result of `std::require(e1, p2)` or `std::prefer(e1, p2)`,

shall compare equal unless

* `p2` is an instance of `S::E`*i*, and
* `p1` and `p2` have different types.

The value of the expression `S::N1::template static_query_v<T>` is

* `T::query(S::N1())`, if that expression is a well-formed expression;
* ill-formed if `declval<T>().query(S::N1())` is well-formed;
* ill-formed if `can_query_v<T, S::N`*i*`>` is `true` for any `1 < ` *i* `<= N`;
* otherwise `S::N1()`.

[*Note:* These rules automatically enable the `S::N1` property by default for objects which do not provide a `query` function for properties `S::N`*i*. *--end note*]

The value of the expression `S::N`*i*`::template static_query_v<T>`, for all `1 < ` *i* `<= N`, is

* `T::query(S::N`*i*`())`, if that expression is a well-formed constant expression;
* otherwise ill-formed.

The value of the expression `S::template static_query_v<T>` is

* `Executor::query(S())`, if that expression is a well-formed constant expression;
* otherwise, ill-formed if `declval<Executor>().query(S())` is well-formed;
* otherwise, `S::N`*i*`::template static_query_v<Executor>` for the least *i* `<= N` for which this expression is a well-formed constant expression;
* otherwise ill-formed.

[*Note:* These rules automatically enable the `S::N1` property by default for objects which do not provide a `query` function for properties `S` or `S::N`*i*. *--end note*]

Let *k* be the least value of *i* for which `can_query_v<Executor,S::N`*i*`>` is true, if such a value of *i* exists.

```
template<class T>
  friend constexpr S query(const T& ex, const Property& p) noexcept(noexcept(std::::query(ex, std::declval<const S::Nk>())));
```

*Returns:* `std::query(ex, S::N`*k*`())`.

*Remarks:* This function shall not participate in overload resolution unless `is_same_v<Property, S> && can_query_v<T, S::N`*i*`>` is true for at least one `S::N`*i*`. 


```
bool operator==(const S& a, const S& b);
```

*Returns:* `true` if `a` and `b` were constructed from the same constructor; `false`, otherwise.