#ifndef STD_EXPERIMENTAL_BITS_IS_TWO_WAY_EXECUTOR_H
#define STD_EXPERIMENTAL_BITS_IS_TWO_WAY_EXECUTOR_H

#include <experimental/bits/is_executor.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace is_two_way_executor_impl {

template<class...>
struct type_check
{
  typedef void type;
};

struct dummy {};

struct nullary_function
{
  dummy operator()() { return {}; }
};

template<class T, class = void>
struct eval : std::false_type {};

template<class T>
struct eval<T,
  typename type_check<
    decltype(static_cast<const dummy&>(std::declval<const T&>().async_execute(std::declval<nullary_function>()).get())),
    decltype(static_cast<const dummy&>(std::declval<const T&>().async_execute(std::declval<nullary_function&>()).get())),
    decltype(static_cast<const dummy&>(std::declval<const T&>().async_execute(std::declval<const nullary_function&>()).get())),
    decltype(static_cast<const dummy&>(std::declval<const T&>().async_execute(std::declval<nullary_function&&>()).get()))
	>::type> : is_executor<T> {};

} // namespace is_two_way_executor_impl

template<class Executor>
struct is_two_way_executor : is_two_way_executor_impl::eval<Executor> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_IS_TWO_WAY_EXECUTOR_H
