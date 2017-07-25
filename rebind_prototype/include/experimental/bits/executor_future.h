#ifndef STD_EXPERIMENTAL_BITS_EXECUTOR_FUTURE_H
#define STD_EXPERIMENTAL_BITS_EXECUTOR_FUTURE_H

#include <type_traits>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

template<class Executor, class T>
struct executor_future
{
  using type = decltype(execution::require(declval<const Executor&>(), execution::twoway).twoway_execute(declval<T(*)()>()));
};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_EXECUTOR_FUTURE_H
