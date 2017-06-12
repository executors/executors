#ifndef STD_EXPERIMENTAL_BITS_REBIND_MEMBER_RESULT_H
#define STD_EXPERIMENTAL_BITS_REBIND_MEMBER_RESULT_H

#include <type_traits>
#include <tuple>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace rebind_member_result_impl {

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
    std::declval<Executor>().rebind(std::declval<Args>()...)
  )>::type>
{
  typedef decltype(std::declval<Executor>().rebind(std::declval<Args>()...)) type;
};

} // namespace rebind_member_result_impl

template<class Executor, class... Args>
struct rebind_member_result : rebind_member_result_impl::eval<Executor, std::tuple<Args...>> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_REBIND_MEMBER_RESULT_H
