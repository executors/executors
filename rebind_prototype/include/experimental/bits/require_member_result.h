#ifndef STD_EXPERIMENTAL_BITS_REQUIRE_MEMBER_RESULT_H
#define STD_EXPERIMENTAL_BITS_REQUIRE_MEMBER_RESULT_H

#include <type_traits>
#include <tuple>
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

template<class Executor, class Args, class = void>
struct eval {};

template<class Executor, class... Args>
struct eval<Executor, std::tuple<Args...>,
  typename type_check<decltype(
    std::declval<Executor>().require(std::declval<Args>()...)
  )>::type>
{
  typedef decltype(std::declval<Executor>().require(std::declval<Args>()...)) type;
};

} // namespace require_member_result_impl

template<class Executor, class... Args>
struct require_member_result : require_member_result_impl::eval<Executor, std::tuple<Args...>> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_REQUIRE_MEMBER_RESULT_H
