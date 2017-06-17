#include <experimental/thread_pool>
#include <iostream>
#include <tuple>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template <class Executor, class Function>
auto async(Executor ex, Function f)
{
  return execution::rebind(ex, execution::twoway).twoway_execute(std::move(f));
}

int main()
{
  static_thread_pool pool{1};
  auto f = async(pool.executor(), []{ return 42; });
  std::cout << "result is " << f.get() << "\n";
}
