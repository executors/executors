#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace custom_hints
{
  struct tracing { bool on; };

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

    // Intercept require and prefer requests for tracing.
    tracing_executor require(custom_hints::tracing t) const { return { t.on, inner_ex_ }; }
    tracing_executor prefer(custom_hints::tracing t) const { return { t.on, inner_ex_ }; }

    // Forward other kinds of require and prefer to the inner executor.
    template <class Property> auto require(const Property& p) const &
      -> tracing_executor<execution::require_member_result_t<InnerExecutor, Property>>
        { return { tracing_, inner_ex_.require(p) }; }
    template <class Property> auto require(const Property& p) &&
      -> tracing_executor<execution::require_member_result_t<InnerExecutor&&, Property>>
        { return { tracing_, std::move(inner_ex_).require(p) }; }
    template <class Property> auto prefer(const Property& p) const &
      -> tracing_executor<execution::prefer_member_result_t<InnerExecutor, Property>>
        { return { tracing_, inner_ex_.prefer(p) }; }
    template <class Property> auto prefer(const Property& p) &&
      -> tracing_executor<execution::prefer_member_result_t<InnerExecutor&&, Property>>
        { return { tracing_, std::move(inner_ex_).prefer(p) }; }

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
    std::enable_if_t<!execution::has_require_member_v<Executor, tracing>, tracing_executor<Executor>>
      require(Executor ex, tracing t) { return { t.on, std::move(ex) }; }
};

class inline_executor
{
public:
  inline_executor require(custom_hints::tracing t) const { inline_executor tmp(*this); tmp.tracing_ = t.on; return tmp; }
  inline_executor prefer(custom_hints::tracing t) const { return require(t); }

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

  auto ex1 = execution::require(inline_executor(), custom_hints::tracing{true});
  ex1.execute([]{ std::cout << "we made it\n"; });

  auto ex2 = execution::prefer(inline_executor(), custom_hints::tracing{true});
  ex2.execute([]{ std::cout << "we made it with a preference\n"; });

  auto ex3 = execution::require(pool.executor(), custom_hints::tracing{true});
  ex3.execute([]{ std::cout << "we made it again\n"; });

  auto ex4 = execution::prefer(pool.executor(), custom_hints::tracing{true});
  ex4.execute([]{ std::cout << "we made it again with a preference\n"; });

  execution::executor ex5 = pool.executor();
  auto ex6 = execution::require(ex5, custom_hints::tracing{true});
  ex6.execute([]{ std::cout << "and again\n"; });

  execution::executor ex7 = pool.executor();
  auto ex8 = execution::prefer(ex7, custom_hints::tracing{true});
  ex8.execute([]{ std::cout << "and again with a preference\n"; });

  pool.wait();
}
