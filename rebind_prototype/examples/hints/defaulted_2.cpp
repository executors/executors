#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace custom_hints
{
  constexpr struct tracing_t {} tracing;

  // Default hint implementation creates an adapter.

  template <class InnerExecutor>
  class tracing_executor
  {
    bool tracing_;
    InnerExecutor inner_ex_;

    template <class T> static auto inner_declval() -> decltype(std::declval<InnerExecutor>());

  public:
    tracing_executor(bool on, const InnerExecutor& ex)
      : tracing_(on), inner_ex_(ex) {}

    // Intercept rebind requests for tracing.
    tracing_executor rebind(custom_hints::tracing_t, bool on) const { return { on, inner_ex_ }; }

    // Forward other kinds of rebind to the inner executor.
    template <class... T> auto rebind(T&&... t) const &
      -> tracing_executor<execution::rebind_member_result_t<InnerExecutor, T...>>
        { return { tracing_, inner_ex_.rebind(std::forward<T>(t)...) }; }
    template <class... T> auto rebind(T&&... t) &&
      -> tracing_executor<execution::rebind_member_result_t<InnerExecutor&&, T...>>
        { return { tracing_, std::move(inner_ex_).rebind(std::forward<T>(t)...) }; }

    auto& context() const noexcept { return inner_ex_.context(); }

    friend bool operator==(const tracing_executor& a, const tracing_executor& b) noexcept
    {
      return a.tracing_ == b.tracing_ && a.inner_ex_ == b.inner_ex_;
    }

    friend bool operator!=(const tracing_executor& a, const tracing_executor& b) noexcept
    {
      return !(a == b);
    }

    template <class Function>
    auto execute(Function f) const
      -> decltype(inner_declval<Function>().execute(std::move(f)))
    {
      return inner_ex_.execute(
          [tracing = tracing_, f = std::move(f)]() mutable
          {
            if (tracing) std::cout << "running function adapted\n";
            return f();
          });
    }

    template <class Function>
    auto twoway_execute(Function f) const
      -> decltype(inner_declval<Function>().twoway_execute(std::move(f)))
    {
      return inner_ex_.twoway_execute(
          [tracing = tracing_, f = std::move(f)]() mutable
          {
            if (tracing) std::cout << "running function adapted\n";
            return f();
          });
    }
  };

  template <class Executor>
    std::enable_if_t<!execution::has_rebind_member_v<Executor, tracing_t, bool>, tracing_executor<Executor>>
      rebind(Executor ex, tracing_t, bool on) { return { on, std::move(ex) }; }
};

class inline_executor
{
public:
  inline_executor rebind(custom_hints::tracing_t, bool on) const { inline_executor tmp(*this); tmp.tracing_ = on; return tmp; }

  auto& context() const noexcept { return *this; }

  friend bool operator==(const inline_executor&, const inline_executor&) noexcept
  {
    return true;
  }

  friend bool operator!=(const inline_executor&, const inline_executor&) noexcept
  {
    return false;
  }

  template <class Function>
  void execute(Function f) const noexcept
  {
    if (tracing_) std::cout << "running function inline\n";
    f();
  }

private:
  bool tracing_;
};

static_assert(execution::is_oneway_executor_v<inline_executor>, "one way executor requirements not met");
static_assert(execution::is_oneway_executor_v<custom_hints::tracing_executor<static_thread_pool::executor_type>>, "one way executor requirements not met");

int main()
{
  static_thread_pool pool{1};

  auto ex1 = execution::rebind(inline_executor(), custom_hints::tracing, true);
  ex1.execute([]{ std::cout << "we made it\n"; });

  auto ex2 = execution::rebind(pool.executor(), custom_hints::tracing, true);
  ex2.execute([]{ std::cout << "we made it again\n"; });

  execution::executor ex3 = pool.executor();
  auto ex4 = execution::rebind(ex3, custom_hints::tracing, true);
  ex4.execute([]{ std::cout << "and again\n"; });

  pool.wait();
}
