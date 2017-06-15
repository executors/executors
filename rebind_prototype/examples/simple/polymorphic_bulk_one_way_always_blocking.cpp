#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{1};
  execution::executor ex = pool.executor().rebind(execution::always_blocking);
  ex.bulk_execute([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 0; });
}
