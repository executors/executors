#include <experimental/thread_pool>
#include <iostream>
#include <tuple>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template <class Executor, class Function, class Args, std::size_t... I>
auto async_helper(Executor ex, Function f, Args args, std::index_sequence<I...>)
{
  return execution::rebind(ex, execution::two_way).async_execute(
      [f = std::move(f), args = std::move(args)]() mutable
      {
        return f(std::move(std::get<I>(args))...);
      });
}

template <class Executor, class Function, class... Args>
auto async(Executor ex, Function f, Args&&... args)
{
  return async_helper(std::move(ex), std::move(f),
      std::make_tuple(std::forward<Args>(args)...),
      std::make_index_sequence<sizeof...(Args)>());
}

int main()
{
  static_thread_pool pool{1};
  auto f = async(pool.executor(), [](int i, int j){ return i + j; }, 20, 22);
  std::cout << "result is " << f.get() << "\n";
}
