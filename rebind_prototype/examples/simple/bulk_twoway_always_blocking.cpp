#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{8};
  auto ex = pool.executor().rebind(execution::always_blocking);
  std::future<int> f = ex.bulk_twoway_execute([](int n, int&, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 42; }, []{ return 0; });
  std::cout << "result is " << f.get() << "\n";
}
