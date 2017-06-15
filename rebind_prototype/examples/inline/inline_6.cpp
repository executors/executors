#include <experimental/execution>
#include <iostream>

namespace execution = std::experimental::execution;

class inline_executor
{
public:
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

static_assert(execution::is_bulk_one_way_executor_v<inline_executor>, "bulk one way executor requirements not met");
static_assert(execution::is_bulk_two_way_executor_v<decltype(execution::rebind(inline_executor(), execution::two_way))>, "bulk two way executor requirements not met");
static_assert(!execution::is_one_way_executor_v<inline_executor>, "must not meet one way executor requirements");
static_assert(!execution::is_two_way_executor_v<decltype(execution::rebind(inline_executor(), execution::two_way))>, "must not meet two way executor requirements");

int main()
{
  inline_executor ex1;
  auto ex2 = execution::rebind(ex1, execution::two_way);
  std::future<void> f1 = ex2.bulk_async_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{}, []{ return 0; });
  f1.wait();
  std::cout << "bulk operation completed\n";
  std::future<int> f2 = ex2.bulk_async_execute([](int n, int&, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 42; }, []{ return 0; });
  f2.wait();
  std::cout << "result is " << f2.get() << "\n";
}
