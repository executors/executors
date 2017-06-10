#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{1};
  auto ex = pool.executor().rebind(execution::always_blocking);
  ex([]{ std::cout << "we made it\n"; });
}
