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

static_assert(execution::is_oneway_executor_v<inline_executor>, "one way executor requirements not met");
static_assert(execution::is_bulk_twoway_executor_v<decltype(execution::require(execution::require(
          inline_executor(), execution::bulk), execution::blocking_adaptable, execution::twoway))>, "bulk two way executor requirements not met");
static_assert(execution::is_bulk_twoway_executor_v<decltype(execution::require(execution::require(
          inline_executor(), execution::blocking_adaptable, execution::twoway), execution::bulk))>, "bulk two way executor requirements not met");

int main()
{
  inline_executor ex1;

  auto ex2 = execution::require(execution::require(ex1, execution::bulk), execution::blocking_adaptable, execution::twoway);
  std::future<void> f1 = ex2.bulk_twoway_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{}, []{ return 0; });
  f1.wait();
  std::cout << "bulk operation is complete\n";
  std::future<int> f2 = ex2.bulk_twoway_execute([](int n, int&, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 42; }, []{ return 0; });
  std::cout << "result is " << f2.get() << "\n";

  auto ex3 = execution::require(execution::require(ex1, execution::blocking_adaptable, execution::twoway), execution::bulk);
  std::future<void> f3 = ex3.bulk_twoway_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{}, []{ return 0; });
  f3.wait();
  std::cout << "bulk operation is complete\n";
  std::future<int> f4 = ex3.bulk_twoway_execute([](int n, int&, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 42; }, []{ return 0; });
  std::cout << "result is " << f4.get() << "\n";
}
