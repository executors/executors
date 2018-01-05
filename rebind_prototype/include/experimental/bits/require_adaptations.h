#ifndef STD_EXPERIMENTAL_BITS_REQUIRE_ADAPTATIONS_H
#define STD_EXPERIMENTAL_BITS_REQUIRE_ADAPTATIONS_H

#include <experimental/bits/require_member_result.h>
#include <future>
#include <type_traits>
#include <tuple>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace require_impl {

// Default require for one way leaves one way executors as is.

template<class Executor>
  constexpr typename std::enable_if<
    (is_oneway_executor<Executor>::value || is_bulk_oneway_executor<Executor>::value)
    && !has_require_member<Executor, oneway_t>::value, Executor>::type
      require(Executor ex, oneway_t) { return std::move(ex); }

// Default require for two way adapts one way executors, leaves two way executors as is.

template<class InnerExecutor>
class twoway_adapter
{
  InnerExecutor inner_ex_;
  template <class T> static auto inner_declval() -> decltype(std::declval<InnerExecutor>());
  template <class, class T> struct dependent_type { using type = T; };

public:
  twoway_adapter(InnerExecutor ex) : inner_ex_(std::move(ex)) {}

  twoway_adapter require(oneway_t) const & { return *this; }
  twoway_adapter require(oneway_t) && { return std::move(*this); }
  twoway_adapter require(twoway_t) const & { return *this; }
  twoway_adapter require(twoway_t) && { return std::move(*this); }

  template<class... T> auto require(T&&... t) const &
    -> twoway_adapter<typename require_member_result<InnerExecutor, T...>::type>
      { return { inner_ex_.require(std::forward<T>(t)...) }; }
  template<class... T> auto require(T&&... t) &&
    -> twoway_adapter<typename require_member_result<InnerExecutor&&, T...>::type>
      { return { std::move(inner_ex_).require(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const twoway_adapter& a, const twoway_adapter& b) noexcept
  {
    return a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const twoway_adapter& a, const twoway_adapter& b) noexcept
  {
    return !(a == b);
  }

  template<class Function> auto execute(Function f) const
    -> decltype(inner_declval<Function>().execute(std::move(f)))
  {
    inner_ex_.execute(std::move(f));
  }

  template<class Function>
  auto twoway_execute(Function f) const -> typename dependent_type<
      decltype(inner_declval<Function>().execute(std::move(f))),
      future<decltype(f())>>::type
  {
    packaged_task<decltype(f())()> task(std::move(f));
    future<decltype(f())> future = task.get_future();
    inner_ex_.execute(std::move(task));
    return future;
  }

  template<class Function, class SharedFactory>
  auto bulk_execute(Function f, std::size_t n, SharedFactory sf) const
    -> decltype(inner_declval<Function>().bulk_execute(std::move(f), n, std::move(sf)))
  {
    inner_ex_.bulk_execute(std::move(f), n, std::move(sf));
  }

  template<class Function, class ResultFactory, class SharedFactory>
  auto bulk_twoway_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const -> typename dependent_type<
      decltype(inner_declval<Function>().bulk_execute(std::move(f), n, std::move(sf))),
      typename std::enable_if<is_same<decltype(rf()), void>::value, future<void>>::type>::type
  {
    auto shared_state = std::make_shared<
        std::tuple<
          std::atomic<std::size_t>, // Number of incomplete functions.
          decltype(sf()), // Underlying shared state.
          std::atomic<std::size_t>, // Number of exceptions raised.
          std::exception_ptr, // First exception raised.
          promise<void> // Promise to receive result
        >>(n, sf(), 0, nullptr, promise<void>());
    future<void> future = std::get<4>(*shared_state).get_future();
    inner_ex_.bulk_execute(
        [f = std::move(f)](auto i, auto& s) mutable
        {
          try
          {
            f(i, std::get<1>(*s));
          }
          catch (...)
          {
            if (std::get<2>(*s)++ == 0)
              std::get<3>(*s) = std::current_exception();
          }
          if (--std::get<0>(*s) == 0)
          {
            if (std::get<3>(*s))
              std::get<4>(*s).set_exception(std::get<3>(*s));
            else
              std::get<4>(*s).set_value();
          }
        }, n, [shared_state]{ return shared_state; });
    return future;
  }

  template<class Function, class ResultFactory, class SharedFactory>
  auto bulk_twoway_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const -> typename dependent_type<
      decltype(inner_declval<Function>().bulk_execute(std::move(f), n, std::move(sf))),
      typename std::enable_if<!is_same<decltype(rf()), void>::value, future<decltype(rf())>>::type>::type
  {
    auto shared_state = std::make_shared<
        std::tuple<
          std::atomic<std::size_t>, // Number of incomplete functions.
          decltype(rf()), // Result.
          decltype(sf()), // Underlying shared state.
          std::atomic<std::size_t>, // Number of exceptions raised.
          std::exception_ptr, // First exception raised.
          promise<decltype(rf())> // Promise to receive result
        >>(n, rf(), sf(), 0, nullptr, promise<decltype(rf())>());
    future<decltype(rf())> future = std::get<5>(*shared_state).get_future();
    inner_ex_.bulk_execute(
        [f = std::move(f)](auto i, auto& s) mutable
        {
          try
          {
            f(i, std::get<1>(*s), std::get<2>(*s));
          }
          catch (...)
          {
            if (std::get<3>(*s)++ == 0)
              std::get<4>(*s) = std::current_exception();
          }
          if (--std::get<0>(*s) == 0)
          {
            if (std::get<4>(*s))
              std::get<5>(*s).set_exception(std::get<4>(*s));
            else
              std::get<5>(*s).set_value(std::move(std::get<1>(*s)));
          }
        }, n, [shared_state]{ return shared_state; });
    return future;
  }
};

template<class Executor>
  typename std::enable_if<
    (is_oneway_executor<Executor>::value || is_bulk_oneway_executor<Executor>::value)
    && !(is_twoway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && has_require_member<Executor, adaptable_blocking_t>::value
    && !has_require_member<Executor, twoway_t>::value, twoway_adapter<Executor>>::type
      require(Executor ex, twoway_t) { return twoway_adapter<Executor>(std::move(ex)); }

template<class Executor>
  constexpr typename std::enable_if<
    (is_twoway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && !has_require_member<Executor, twoway_t>::value, Executor>::type
      require(Executor ex, twoway_t) { return std::move(ex); }

// Default adapter for unsafe mode adds the unsafe mode property.

template<class InnerExecutor>
class adaptable_blocking_adapter
{
  InnerExecutor inner_ex_;
  template <class T> static auto inner_declval() -> decltype(std::declval<InnerExecutor>());

public:
  adaptable_blocking_adapter(InnerExecutor ex) : inner_ex_(std::move(ex)) {}

  adaptable_blocking_adapter require(adaptable_blocking_t) const & { return *this; }
  adaptable_blocking_adapter require(adaptable_blocking_t) && { return std::move(*this); }
  InnerExecutor require(not_adaptable_blocking_t) const & { return inner_ex_; }
  InnerExecutor require(not_adaptable_blocking_t) && { return std::move(inner_ex_); }

  template<class... T> auto require(T&&... t) const &
    -> adaptable_blocking_adapter<typename require_member_result<InnerExecutor, T...>::type>
      { return { inner_ex_.require(std::forward<T>(t)...) }; }
  template<class... T> auto require(T&&... t) &&
    -> adaptable_blocking_adapter<typename require_member_result<InnerExecutor&&, T...>::type>
      { return { std::move(inner_ex_).require(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const adaptable_blocking_adapter& a, const adaptable_blocking_adapter& b) noexcept
  {
    return a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const adaptable_blocking_adapter& a, const adaptable_blocking_adapter& b) noexcept
  {
    return !(a == b);
  }

  template<class Function> auto execute(Function f) const
    -> decltype(inner_declval<Function>().execute(std::move(f)))
  {
    return inner_ex_.execute(std::move(f));
  }

  template<class Function>
  auto twoway_execute(Function f) const
    -> decltype(inner_declval<Function>().twoway_execute(std::move(f)))
  {
    return inner_ex_.twoway_execute(std::move(f));
  }

  template<class Function, class SharedFactory>
  auto bulk_execute(Function f, std::size_t n, SharedFactory sf) const
    -> decltype(inner_declval<Function>().bulk_execute(std::move(f), n, std::move(sf)))
  {
    return inner_ex_.bulk_execute(std::move(f), n, std::move(sf));
  }

  template<class Function, class ResultFactory, class SharedFactory>
  auto bulk_twoway_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const
    -> decltype(inner_declval<Function>().bulk_twoway_execute(std::move(f), n, std::move(rf), std::move(sf)))
  {
    return inner_ex_.bulk_twoway_execute(std::move(f), n, std::move(rf), std::move(sf));
  }
};

template<class Executor>
  constexpr typename std::enable_if<!has_require_member<Executor, adaptable_blocking_t>::value,
    adaptable_blocking_adapter<Executor>>::type
      require(Executor ex, adaptable_blocking_t) { return adaptable_blocking_adapter<Executor>(std::move(ex)); }

// Default require for bulk adapts single executors, leaves bulk executors as is.

template<class InnerExecutor>
class bulk_adapter
{
  InnerExecutor inner_ex_;
  template <class T> static auto inner_declval() -> decltype(std::declval<InnerExecutor>());
  template <class, class T> struct dependent_type { using type = T; };

public:
  bulk_adapter(InnerExecutor ex) : inner_ex_(std::move(ex)) {}

  bulk_adapter require(single_t) const & { return *this; }
  bulk_adapter require(single_t) && { return std::move(*this); }
  bulk_adapter require(bulk_t) const & { return *this; }
  bulk_adapter require(bulk_t) && { return std::move(*this); }

  template<class... T> auto require(T&&... t) const &
    -> bulk_adapter<typename require_member_result<InnerExecutor, T...>::type>
      { return { inner_ex_.require(std::forward<T>(t)...) }; }
  template<class... T> auto require(T&&... t) &&
    -> bulk_adapter<typename require_member_result<InnerExecutor&&, T...>::type>
      { return { std::move(inner_ex_).require(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const bulk_adapter& a, const bulk_adapter& b) noexcept
  {
    return a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const bulk_adapter& a, const bulk_adapter& b) noexcept
  {
    return !(a == b);
  }

  template<class Function> void execute(Function f) const
  {
    inner_ex_.execute(std::move(f));
  }

  template<class Function>
  auto twoway_execute(Function f) const
    -> decltype(inner_declval<Function>().twoway_execute(std::move(f)))
  {
    return inner_ex_.twoway_execute(std::move(f));
  }

  template<class Function, class SharedFactory>
  void bulk_execute(Function f, std::size_t n, SharedFactory sf) const
  {
    auto shared_state = std::make_shared<decltype(sf())>(sf());
    for (std::size_t i = 0; i < n; ++i)
    {
      inner_ex_.execute(
          [f = std::move(f), i, shared_state]() mutable
          {
            f(i, *shared_state);
          });
    }
  }

  template<class Function, class ResultFactory, class SharedFactory>
  auto bulk_twoway_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const -> typename dependent_type<
      decltype(inner_declval<Function>().twoway_execute(std::move(rf))),
      typename std::enable_if<is_same<decltype(rf()), void>::value, future<void>>::type>::type
  {
    auto shared_state = std::make_shared<
        std::tuple<
          std::atomic<std::size_t>, // Number of incomplete functions.
          decltype(sf()), // Underlying shared state.
          std::atomic<std::size_t>, // Number of exceptions raised.
          std::exception_ptr, // First exception raised.
          promise<void> // Promise to receive result
        >>(n, sf(), 0, nullptr, promise<void>());
    future<void> future = std::get<4>(*shared_state).get_future();
    this->bulk_execute(
        [f = std::move(f)](auto i, auto& s) mutable
        {
          try
          {
            f(i, std::get<1>(*s));
          }
          catch (...)
          {
            if (std::get<2>(*s)++ == 0)
              std::get<3>(*s) = std::current_exception();
          }
          if (--std::get<0>(*s) == 0)
          {
            if (std::get<3>(*s))
              std::get<4>(*s).set_exception(std::get<3>(*s));
            else
              std::get<4>(*s).set_value();
          }
        }, n, [shared_state]{ return shared_state; });
    return future;
  }

  template<class Function, class ResultFactory, class SharedFactory>
  auto bulk_twoway_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const -> typename dependent_type<
      decltype(inner_declval<Function>().twoway_execute(std::move(rf))),
      typename std::enable_if<!is_same<decltype(rf()), void>::value, future<decltype(rf())>>::type>::type
  {
    auto shared_state = std::make_shared<
        std::tuple<
          std::atomic<std::size_t>, // Number of incomplete functions.
          decltype(rf()), // Result.
          decltype(sf()), // Underlying shared state.
          std::atomic<std::size_t>, // Number of exceptions raised.
          std::exception_ptr, // First exception raised.
          promise<decltype(rf())> // Promise to receive result
        >>(n, rf(), sf(), 0, nullptr, promise<decltype(rf())>());
    future<decltype(rf())> future = std::get<5>(*shared_state).get_future();
    this->bulk_execute(
        [f = std::move(f)](auto i, auto& s) mutable
        {
          try
          {
            f(i, std::get<1>(*s), std::get<2>(*s));
          }
          catch (...)
          {
            if (std::get<3>(*s)++ == 0)
              std::get<4>(*s) = std::current_exception();
          }
          if (--std::get<0>(*s) == 0)
          {
            if (std::get<4>(*s))
              std::get<5>(*s).set_exception(std::get<4>(*s));
            else
              std::get<5>(*s).set_value(std::move(std::get<1>(*s)));
          }
        }, n, [shared_state]{ return shared_state; });
    return future;
  }
};

template<class Executor>
  typename std::enable_if<is_oneway_executor<Executor>::value
    && !(is_bulk_oneway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && !has_require_member<Executor, bulk_t>::value, bulk_adapter<Executor>>::type
      require(Executor ex, bulk_t) { return bulk_adapter<Executor>(std::move(ex)); }

template<class Executor>
  constexpr typename std::enable_if<
    (is_bulk_oneway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && !has_require_member<Executor, bulk_t>::value, Executor>::type
      require(Executor ex, bulk_t) { return std::move(ex); }

// Default require for always blocking adapts all executors.

template<class InnerExecutor>
class always_blocking_adapter
{
  InnerExecutor inner_ex_;
  template <class T> static auto inner_declval() -> decltype(std::declval<InnerExecutor>());

public:
  always_blocking_adapter(InnerExecutor ex) : inner_ex_(std::move(ex)) {}

  always_blocking_adapter require(always_blocking_t) const & { return *this; }
  always_blocking_adapter require(always_blocking_t) && { return std::move(*this); }
  always_blocking_adapter require(possibly_blocking_t) const & { return *this; }
  always_blocking_adapter require(possibly_blocking_t) && { return std::move(*this); }

  template<class... T> auto require(T&&... t) const &
    -> always_blocking_adapter<typename require_member_result<InnerExecutor, T...>::type>
      { return { inner_ex_.require(std::forward<T>(t)...) }; }
  template<class... T> auto require(T&&... t) &&
    -> always_blocking_adapter<typename require_member_result<InnerExecutor&&, T...>::type>
      { return { std::move(inner_ex_).require(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const always_blocking_adapter& a, const always_blocking_adapter& b) noexcept
  {
    return a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const always_blocking_adapter& a, const always_blocking_adapter& b) noexcept
  {
    return !(a == b);
  }

  template<class Function> auto execute(Function f) const
    -> decltype(inner_declval<Function>().execute(std::move(f)))
  {
    promise<void> promise;
    future<void> future = promise.get_future();
    inner_ex_.execute([f = std::move(f), p = std::move(promise)]() mutable { f(); });
    future.wait();
  }

  template<class Function>
  auto twoway_execute(Function f) const
    -> decltype(inner_declval<Function>().twoway_execute(std::move(f)))
  {
    auto future = inner_ex_.twoway_execute(std::move(f));
    future.wait();
    return future;
  }

  template<class Function, class SharedFactory>
  auto bulk_execute(Function f, std::size_t n, SharedFactory sf) const
    -> decltype(inner_declval<Function>().bulk_execute(std::move(f), n, std::move(sf)))
  {
    promise<void> promise;
    future<void> future = promise.get_future();
    inner_ex_.bulk_execute([f = std::move(f), p = std::move(promise)](auto i, auto& s) mutable { f(i, s); }, n, std::move(sf));
    future.wait();
  }

  template<class Function, class ResultFactory, class SharedFactory>
  auto bulk_twoway_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const
    -> decltype(inner_declval<Function>().bulk_twoway_execute(std::move(f), n, std::move(rf), std::move(sf)))
  {
    auto future = inner_ex_.bulk_twoway_execute(std::move(f), n, std::move(rf), std::move(sf));
    future.wait();
    return future;
  }
};

template<class Executor>
  constexpr typename std::enable_if<!has_require_member<Executor, always_blocking_t>::value,
    always_blocking_adapter<Executor>>::type
      require(Executor ex, always_blocking_t) { return always_blocking_adapter<Executor>(std::move(ex)); }

// Default require for possibly blocking does no adaptation, as all executors are possibly blocking.

template<class Executor>
  constexpr typename std::enable_if<!has_require_member<Executor, possibly_blocking_t>::value, Executor>::type
    require(Executor ex, possibly_blocking_t) { return std::move(ex); }

} // namespace require_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_REQUIRE_ADAPTATIONS_H
