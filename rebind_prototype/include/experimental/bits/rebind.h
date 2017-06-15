#ifndef STD_EXPERIMENTAL_BITS_REBIND_H
#define STD_EXPERIMENTAL_BITS_REBIND_H

#include <experimental/bits/has_rebind_member.h>
#include <experimental/bits/is_two_way_executor.h>
#include <experimental/bits/is_one_way_executor.h>
#include <experimental/bits/is_bulk_one_way_executor.h>
#include <experimental/bits/is_bulk_two_way_executor.h>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

struct one_way_t;
struct two_way_t;
struct single_t;
struct bulk_t;
struct always_blocking_t;
struct possibly_blocking_t;
struct is_continuation_t;
struct is_not_continuation_t;
struct is_work_t;
struct is_not_work_t;

namespace rebind_impl {

template<class Executor, class... Args>
constexpr auto rebind(Executor&& ex, Args&&... args)
  -> decltype(std::forward<Executor>(ex).rebind(std::forward<Args>(args)...))
{
  return std::forward<Executor>(ex).rebind(std::forward<Args>(args)...);
}

// Forward declare the default adaptations.
template<class Executor>
  constexpr typename std::enable_if<
    (is_one_way_executor<Executor>::value || is_bulk_one_way_executor<Executor>::value)
    && !has_rebind_member<Executor, one_way_t>::value, Executor>::type
      rebind(Executor ex, one_way_t);
template<class Executor> class two_way_adapter;
template<class Executor>
  typename std::enable_if<
    (is_one_way_executor<Executor>::value || is_bulk_one_way_executor<Executor>::value)
    && !(is_two_way_executor<Executor>::value || is_bulk_two_way_executor<Executor>::value)
    && !has_rebind_member<Executor, two_way_t>::value, two_way_adapter<Executor>>::type
      rebind(Executor ex, two_way_t);
template<class Executor> class bulk_adapter;
template<class Executor>
  constexpr typename std::enable_if<
    (is_two_way_executor<Executor>::value || is_bulk_two_way_executor<Executor>::value)
    && !has_rebind_member<Executor, two_way_t>::value, Executor>::type
      rebind(Executor ex, two_way_t);
template<class Executor>
  typename std::enable_if<is_one_way_executor<Executor>::value
    && !(is_bulk_one_way_executor<Executor>::value || is_bulk_two_way_executor<Executor>::value)
    && !has_rebind_member<Executor, bulk_t>::value, bulk_adapter<Executor>>::type
      rebind(Executor ex, bulk_t);
template<class Executor>
  constexpr typename std::enable_if<
    (is_bulk_one_way_executor<Executor>::value || is_bulk_two_way_executor<Executor>::value)
    && !has_rebind_member<Executor, bulk_t>::value, Executor>::type
      rebind(Executor ex, bulk_t);
template<class Executor> class always_blocking_adapter;
template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, always_blocking_t>::value,
    always_blocking_adapter<Executor>>::type
      rebind(Executor ex, always_blocking_t);
template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, possibly_blocking_t>::value, Executor>::type
    rebind(Executor ex, possibly_blocking_t);
template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_continuation_t>::value, Executor>::type
    rebind(Executor ex, is_continuation_t);
template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_not_continuation_t>::value, Executor>::type
    rebind(Executor ex, is_not_continuation_t);
template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_work_t>::value, Executor>::type
    rebind(Executor ex, is_work_t);
template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_not_work_t>::value, Executor>::type
    rebind(Executor ex, is_not_work_t);
template<class Executor, class ProtoAllocator>
  constexpr typename std::enable_if<!has_rebind_member<Executor, std::allocator_arg_t, ProtoAllocator>::value, Executor>::type
    rebind(Executor ex, std::allocator_arg_t, const ProtoAllocator&);

struct rebind_fn
{
  template<class Executor, class... Args>
  constexpr auto operator()(Executor&& ex, Args&&... args) const
    noexcept(noexcept(rebind(std::forward<Executor>(ex), std::forward<Args>(args)...)))
    -> decltype(rebind(std::forward<Executor>(ex), std::forward<Args>(args)...))
  {
    return rebind(std::forward<Executor>(ex), std::forward<Args>(args)...);
  }
};

template<class T = rebind_fn> constexpr T customization_point{};

} // namespace rebind_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_REBIND_H
