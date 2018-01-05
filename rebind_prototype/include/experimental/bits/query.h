#ifndef STD_EXPERIMENTAL_BITS_QUERY_H
#define STD_EXPERIMENTAL_BITS_QUERY_H

#include <experimental/bits/has_query_member.h>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

namespace query_impl {

template<class Executor, class Property>
constexpr auto query(Executor&& ex, Property&& p)
  -> decltype(std::forward<Executor>(ex).query(std::forward<Property>(p)))
{
  return std::forward<Executor>(ex).query(std::forward<Property>(p));
}

struct query_fn
{
  template<class Executor, class Property>
  constexpr auto operator()(Executor&& ex, Property&& p) const
    noexcept(noexcept(query(std::forward<Executor>(ex), std::forward<Property>(p))))
    -> decltype(query(std::forward<Executor>(ex), std::forward<Property>(p)))
  {
    return query(std::forward<Executor>(ex), std::forward<Property>(p));
  }
};

template<class T = query_fn> constexpr T customization_point{};

} // namespace query_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_QUERY_H
