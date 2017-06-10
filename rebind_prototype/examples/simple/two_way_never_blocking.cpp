#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{1};
  auto ex = pool.executor().rebind(execution::two_way).rebind(execution::never_blocking);
  std::future<int> f = ex([]{ return 42; });
  std::cout << "result is " << f.get() << "\n";
}
