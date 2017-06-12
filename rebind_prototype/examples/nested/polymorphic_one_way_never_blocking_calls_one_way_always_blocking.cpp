#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{1};
  execution::one_way_executor ex = pool.executor().rebind(execution::never_blocking);
  std::cout << "before submission\n";
  ex([ex = ex.rebind(execution::always_blocking)]{
      std::cout << "outer starts\n";
      ex([]{ std::cout << "inner\n"; });
      std::cout << "outer ends\n";
    });
  std::cout << "after submission, before wait\n";
  pool.wait();
  std::cout << "after wait\n";
}
