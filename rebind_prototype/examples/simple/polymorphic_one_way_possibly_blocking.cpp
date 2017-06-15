#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{1};
  execution::one_way_executor ex = pool.executor().rebind(execution::possibly_blocking);
  ex.execute([]{ std::cout << "we made it\n"; });
  pool.wait();
}
