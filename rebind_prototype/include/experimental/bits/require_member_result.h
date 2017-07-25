#ifndef STD_EXPERIMENTAL_BITS_REQUIRE_MEMBER_RESULT_H
#define STD_EXPERIMENTAL_BITS_REQUIRE_MEMBER_RESULT_H

#include <type_traits>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace require_member_result_impl {

template<class>
struct type_check
{
  typedef void type;
};

template<class Executor, class Property, class = void>
struct eval {};

template<class Executor, class Property>
struct eval<Executor, Property,
  typename type_check<decltype(
    std::declval<Executor>().require(std::declval<Property>())
  )>::type>
{
  typedef decltype(std::declval<Executor>().require(std::declval<Property>())) type;
};

} // namespace require_member_result_impl

template<class Executor, class Property>
struct require_member_result : require_member_result_impl::eval<Executor, Property> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_REQUIRE_MEMBER_RESULT_H
