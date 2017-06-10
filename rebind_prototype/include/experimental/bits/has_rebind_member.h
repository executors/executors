#ifndef STD_EXPERIMENTAL_BITS_HAS_REBIND_MEMBER_H
#define STD_EXPERIMENTAL_BITS_HAS_REBIND_MEMBER_H

#include <type_traits>
#include <tuple>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace has_rebind_member_impl {

template<class>
struct type_check
{
  typedef void type;
};

template<class Executor, class Args, class = void>
struct eval : std::false_type {};

template<class Executor, class... Args>
struct eval<Executor, std::tuple<Args...>,
  typename type_check<decltype(
    std::declval<Executor>().rebind(std::declval<Args>()...)
  )>::type> : std::true_type {};

} // namespace has_rebind_member_impl

template<class Executor, class... Args>
struct has_rebind_member : has_rebind_member_impl::eval<Executor, std::tuple<Args...>> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_HAS_REBIND_MEMBER_H
