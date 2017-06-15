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

  template <class Function>
  void execute(Function f) const noexcept
  {
    f();
  }
};

static_assert(execution::is_one_way_executor_v<inline_executor>, "one way executor requirements not met");
static_assert(execution::is_bulk_two_way_executor_v<decltype(execution::rebind(execution::rebind(inline_executor(), execution::bulk), execution::two_way))>, "bulk two way executor requirements not met");
static_assert(execution::is_bulk_two_way_executor_v<decltype(execution::rebind(execution::rebind(inline_executor(), execution::two_way), execution::bulk))>, "bulk two way executor requirements not met");

int main()
{
  inline_executor ex1;

  auto ex2 = execution::rebind(execution::rebind(ex1, execution::bulk), execution::two_way);
  std::future<void> f1 = ex2.bulk_async_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{}, []{ return 0; });
  f1.wait();
  std::cout << "bulk operation is complete\n";
  std::future<int> f2 = ex2.bulk_async_execute([](int n, int&, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 42; }, []{ return 0; });
  std::cout << "result is " << f2.get() << "\n";

  auto ex3 = execution::rebind(execution::rebind(ex1, execution::two_way), execution::bulk);
  std::future<void> f3 = ex3.bulk_async_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{}, []{ return 0; });
  f3.wait();
  std::cout << "bulk operation is complete\n";
  std::future<int> f4 = ex3.bulk_async_execute([](int n, int&, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 42; }, []{ return 0; });
  std::cout << "result is " << f4.get() << "\n";
}
