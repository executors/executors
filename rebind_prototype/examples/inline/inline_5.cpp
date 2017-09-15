#include <experimental/execution>
#include <iostream>

namespace execution = std::experimental::execution;

class inline_executor
{
public:
  inline_executor require(execution::always_blocking_t) const { return *this; }
  inline_executor require(execution::possibly_blocking_t) const { return *this; }

  auto& context() const noexcept { return *this; }

  friend bool operator==(const inline_executor&, const inline_executor&) noexcept
  {
    return true;
  }

  friend bool operator!=(const inline_executor&, const inline_executor&) noexcept
  {
    return false;
  }

  template <class Function, class SharedFactory>
  void bulk_execute(Function f, std::size_t n, SharedFactory sf) const noexcept
  {
    auto shared_state(sf());
    for (std::size_t i = 0; i < n; ++i)
      f(i, shared_state);
  }
};

static_assert(execution::is_bulk_oneway_executor_v<inline_executor>, "bulk one way executor requirements not met");
static_assert(!execution::is_oneway_executor_v<inline_executor>, "must not meet one way executor requirements");

int main()
{
  inline_executor ex1;
  auto ex2 = ex1.require(execution::always_blocking);
  ex2.bulk_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 0; });
}
