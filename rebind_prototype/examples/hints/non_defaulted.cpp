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

static_assert(execution::is_one_way_executor_v<inline_executor>, "one way executor requirements not met");

int main()
{
  auto ex1 = execution::rebind(inline_executor(), custom_hints::tracing, true);
  ex1.execute([]{ std::cout << "we made it\n"; });

  // No default means we can't rebind arbitrary executors using our custom hint.
  static_assert(!execution::can_rebind_v<static_thread_pool::executor_type, custom_hints::tracing_t, bool>, "can't add tracing to static_thread_pool");
}
