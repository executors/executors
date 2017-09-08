#ifndef STD_EXPERIMENTAL_BITS_HAS_REQUIRE_MEMBERS_H
#define STD_EXPERIMENTAL_BITS_HAS_REQUIRE_MEMBERS_H

#include <type_traits>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace has_require_members_impl {

template<class>
struct type_check
{
  typedef void type;
};

template<class Executor, class Property, class = void>
struct eval_one : std::false_type {};

template<class Executor, class Property>
struct eval_one<Executor, Property,
  typename type_check<decltype(
    std::declval<Executor>().require(std::declval<Property>())
  )>::type> : std::true_type {};

template<class Executor, class... Properties>
struct eval;

template<class Executor, class Property, class... Properties>
struct eval<Executor, Property, Properties...> :
  std::integral_constant<bool,
    eval_one<Executor, Property>::value
      && eval<Executor, Properties...>::value> {};

template<class Executor, class Property>
struct eval<Executor, Property> : eval_one<Executor, Property> {};

template<class Executor>
struct eval<Executor> : std::true_type {};

} // namespace has_require_members_impl

template<class Executor, class... Properties>
struct has_require_members : has_require_members_impl::eval<Executor, Properties...> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_HAS_REQUIRE_MEMBERS_H
