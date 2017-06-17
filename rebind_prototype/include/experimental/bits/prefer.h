#ifndef STD_EXPERIMENTAL_BITS_PREFER_H
#define STD_EXPERIMENTAL_BITS_PREFER_H

#include <experimental/bits/has_prefer_member.h>
#include <experimental/bits/has_require_member.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

namespace prefer_impl {

template<class Executor, class Property, class... Args>
constexpr auto prefer(Executor&& ex, Property&& p, Args&&... args)
  -> decltype(std::forward<Executor>(ex).prefer(std::forward<Property>(p), std::forward<Args>(args)...))
{
  return std::forward<Executor>(ex).prefer(std::forward<Property>(p), std::forward<Args>(args)...);
}

template<class Executor, class...Args>
constexpr auto prefer(Executor ex, Args&&... args)
  -> typename std::enable_if<!has_prefer_member<Executor, Args...>::value,
    decltype(std::forward<Executor>(ex).require(std::forward<Args>(args)...))>::type
{
  return std::forward<Executor>(ex).require(std::forward<Args>(args)...);
}

template<class Executor, class...Args>
constexpr auto prefer(Executor ex, Args&&...)
  -> typename std::enable_if<!has_prefer_member<Executor, Args...>::value
    && !has_require_member<Executor, Args...>::value, Executor>::type
{
  return ex;
}

struct prefer_fn
{
  template<class Executor, class Property, class... Args>
  constexpr auto operator()(Executor&& ex, Property&& p, Args&&... args) const
    noexcept(noexcept(prefer(std::forward<Executor>(ex), std::forward<Property>(p), std::forward<Args>(args)...)))
    -> decltype(prefer(std::forward<Executor>(ex), std::forward<Property>(p), std::forward<Args>(args)...))
  {
    return prefer(std::forward<Executor>(ex), std::forward<Property>(p), std::forward<Args>(args)...);
  }
};

template<class T = prefer_fn> constexpr T customization_point{};

} // namespace prefer_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_PREFER_H
