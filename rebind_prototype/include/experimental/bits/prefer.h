#ifndef STD_EXPERIMENTAL_BITS_PREFER_H
#define STD_EXPERIMENTAL_BITS_PREFER_H

#include <experimental/bits/has_prefer_member.h>
#include <experimental/bits/has_require_member.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

namespace prefer_impl {

template<class Executor, class Property>
constexpr auto prefer(Executor&& ex, Property&& p)
  -> decltype(std::forward<Executor>(ex).prefer(std::forward<Property>(p)))
{
  return std::forward<Executor>(ex).prefer(std::forward<Property>(p));
}

template<class Executor, class Property>
constexpr auto prefer(Executor ex, Property&& p)
  -> typename std::enable_if<!has_prefer_member<Executor, Property>::value,
    decltype(std::forward<Executor>(ex).require(std::forward<Property>(p)))>::type
{
  return std::forward<Executor>(ex).require(std::forward<Property>(p));
}

template<class Executor, class Property>
constexpr auto prefer(Executor ex, Property&&)
  -> typename std::enable_if<!has_prefer_member<Executor, Property>::value
    && !has_require_member<Executor, Property>::value, Executor>::type
{
  return ex;
}

struct prefer_fn
{
  template<class Executor, class Property>
  constexpr auto operator()(Executor&& ex, Property&& p) const
    noexcept(noexcept(prefer(std::forward<Executor>(ex), std::forward<Property>(p))))
    -> decltype(prefer(std::forward<Executor>(ex), std::forward<Property>(p)))
  {
    return prefer(std::forward<Executor>(ex), std::forward<Property>(p));
  }

  template<class Executor, class Property0, class Property1, class... PropertyN>
  constexpr auto operator()(Executor&& ex, Property0&& p0, Property1&& p1, PropertyN&&... pn) const
    noexcept(noexcept(std::declval<prefer_fn>()(std::declval<prefer_fn>()(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...)))
    -> decltype(std::declval<prefer_fn>()(std::declval<prefer_fn>()(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...))
  {
    return (*this)((*this)(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...);
  }
};

template<class T = prefer_fn> constexpr T customization_point{};

} // namespace prefer_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_PREFER_H
