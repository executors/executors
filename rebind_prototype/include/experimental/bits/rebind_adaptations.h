#ifndef STD_EXPERIMENTAL_BITS_REBIND_ADAPTATIONS_H
#define STD_EXPERIMENTAL_BITS_REBIND_ADAPTATIONS_H

#include <experimental/bits/rebind_member_result.h>
#include <future>
#include <type_traits>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace rebind_impl {

// Default rebind for one way leaves one way executors as is.

template<class Executor>
  constexpr typename std::enable_if<is_one_way_executor<Executor>::value
    && !has_rebind_member<Executor, one_way_t>::value, Executor>::type
      rebind(Executor ex, one_way_t) { return std::move(ex); }

// Default rebind for two way adapts one way executors, leaves two way executors as is.

template <class InnerExecutor>
class two_way_adapter
{
  InnerExecutor inner_ex_;

public:
  two_way_adapter(InnerExecutor ex) : inner_ex_(std::move(ex)) {}

  InnerExecutor rebind(one_way_t) const & { return inner_ex_; }
  InnerExecutor rebind(one_way_t) && { return std::move(inner_ex_); }
  two_way_adapter rebind(two_way_t) const & { return *this; }
  two_way_adapter rebind(two_way_t) && { return std::move(*this); }

  template <class... T> auto rebind(T&&... t) const &
    -> two_way_adapter<typename rebind_member_result<InnerExecutor, T...>::type>
      { return { inner_ex_.rebind(std::forward<T>(t)...) }; }
  template <class... T> auto rebind(T&&... t) &&
    -> two_way_adapter<typename rebind_member_result<InnerExecutor&&, T...>::type>
      { return { std::move(inner_ex_).rebind(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const two_way_adapter& a, const two_way_adapter& b) noexcept
  {
    return a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const two_way_adapter& a, const two_way_adapter& b) noexcept
  {
    return !(a == b);
  }

  template <class Function>
  auto operator()(Function f) const -> std::future<decltype(f())>
  {
    std::packaged_task<decltype(f())()> task(std::move(f));
    std::future<decltype(f())> future = task.get_future();
    inner_ex_(std::move(task));
    return future;
  }
};

template<class Executor>
  typename std::enable_if<is_one_way_executor<Executor>::value && !is_two_way_executor<Executor>::value
    && !has_rebind_member<Executor, two_way_t>::value, two_way_adapter<Executor>>::type
      rebind(Executor ex, two_way_t) { return two_way_adapter<Executor>(std::move(ex)); }

template<class Executor>
  constexpr typename std::enable_if<is_two_way_executor<Executor>::value
    && !has_rebind_member<Executor, two_way_t>::value, Executor>::type
      rebind(Executor ex, two_way_t) { return std::move(ex); }

// Default rebind for possibly blocking does no adaptation, as all executors are possibly blocking.

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, possibly_blocking_t>::value, Executor>::type
    rebind(Executor ex, possibly_blocking_t) { return std::move(ex); }

// Default rebind for continuation mode does no adaptation.

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_continuation_t>::value, Executor>::type
    rebind(Executor ex, is_continuation_t) { return std::move(ex); }

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_not_continuation_t>::value, Executor>::type
    rebind(Executor ex, is_not_continuation_t) { return std::move(ex); }

// Default rebind for work mode does no adaptation.

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_work_t>::value, Executor>::type
    rebind(Executor ex, is_work_t) { return std::move(ex); }

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_not_work_t>::value, Executor>::type
    rebind(Executor ex, is_not_work_t) { return std::move(ex); }

// Default rebind for allocator does no adaptation.

template<class Executor, class ProtoAllocator>
  constexpr typename std::enable_if<!has_rebind_member<Executor, std::allocator_arg_t, ProtoAllocator>::value, Executor>::type
    rebind(Executor ex, std::allocator_arg_t, const ProtoAllocator&) { return std::move(ex); }

} // namespace rebind_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_REBIND_ADAPTATIONS_H
