#ifndef STD_EXPERIMENTAL_BITS_EXECUTOR_CONTEXT_H
#define STD_EXPERIMENTAL_BITS_EXECUTOR_CONTEXT_H

#include <type_traits>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

template<class Executor>
struct executor_context
{
  using type = std::decay_t<decltype(declval<const Executor&>().context())>;
};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_EXECUTOR_CONTEXT_H
