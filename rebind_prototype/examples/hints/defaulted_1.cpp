#include <experimental/thread_pool>
#include <cassert>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace custom_hints
{
  struct tracing { bool on = false; };

  // Default hint implementation drops it.
  template <class Executor>
    std::enable_if_t<!execution::has_require_member_v<Executor, tracing>, Executor>
      require(Executor ex, tracing) { return std::move(ex); }
};

class inline_executor
{
public:
  inline_executor require(custom_hints::tracing t) const { inline_executor tmp(*this); tmp.tracing_ = t.on; return tmp; }

  bool query(custom_hints::tracing) const { return tracing_; }

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

  auto ex1 = execution::require(inline_executor(), custom_hints::tracing{true});
  assert(execution::query(ex1, custom_hints::tracing{}));
  ex1.execute([]{ std::cout << "we made it\n"; });

  auto ex2 = execution::require(pool.executor(), custom_hints::tracing{true});
  static_assert(!execution::can_query_v<decltype(ex2), custom_hints::tracing>, "cannot query tracing for static_thread_pool::executor");
  ex2.execute([]{ std::cout << "we made it again\n"; });

  pool.wait();
}
