#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace custom_hints
{
  constexpr struct tracing_t {} tracing;
};

class inline_executor
{
public:
  inline_executor require(custom_hints::tracing_t, bool on) const { inline_executor tmp(*this); tmp.tracing_ = on; return tmp; }

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

int main()
{
  static_thread_pool pool{1};

  auto ex1 = execution::require(inline_executor(), custom_hints::tracing, true);
  ex1.execute([]{ std::cout << "we made it\n"; });

  auto ex2 = execution::prefer(inline_executor(), custom_hints::tracing, true);
  ex2.execute([]{ std::cout << "we made it with a preference\n"; });

  // No default means we can't require arbitrary executors using our custom hint ...
  static_assert(!execution::can_require_v<static_thread_pool::executor_type, custom_hints::tracing_t, bool>, "can't require tracing from static_thread_pool");

  // ... but we can still ask for it as a preference.
  auto ex3 = execution::prefer(pool.executor(), custom_hints::tracing, true);
  ex3.execute([]{ std::cout << "we made it again with a preference\n"; });
  pool.wait();
}
