#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;
using std::experimental::executors_v1::future;

int main()
{
  static_thread_pool pool{1};
  auto ex = pool.executor();

  // One way, single.
  ex.execute([]{ std::cout << "we made it\n"; });

  // Two way, single.
  future<int> f1 = ex.twoway_execute([]{ return 42; });
  f1.wait();
  std::cout << "result is " << f1.get() << "\n";

  // One way, bulk.
  ex.bulk_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 0; });

  // Two way, bulk, void result.
  future<void> f2 = ex.bulk_twoway_execute(
      [](int n, int&)
      {
        std::cout << "async part " << n << "\n";
      }, 8, []{}, []{ return 0; });
  f2.wait();
  std::cout << "bulk result available\n";

  // Two way, bulk, non-void result.
  future<double> f3 = ex.bulk_twoway_execute(
      [](int n, double&, int&)
      {
        std::cout << "async part " << n << "\n";
      }, 8, [](){ return 123.456; }, []{ return 0; });
  f3.wait();
  std::cout << "bulk result is " << f3.get() << "\n";
}
