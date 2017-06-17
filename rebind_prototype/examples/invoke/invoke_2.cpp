#include <experimental/thread_pool>
#include <iostream>
#include <tuple>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template <class Executor, class Function, class Args, std::size_t... I>
auto invoke_helper(Executor ex, Function f, Args args, std::index_sequence<I...>)
{
  return execution::prefer(execution::require(ex, execution::twoway), execution::always_blocking).twoway_execute(
      [f = std::move(f), args = std::move(args)]() mutable
      {
        return f(std::move(std::get<I>(args))...);
      }).get();
}

template <class Executor, class Function, class... Args>
auto invoke(Executor ex, Function f, Args&&... args)
{
  return invoke_helper(std::move(ex), std::move(f),
      std::make_tuple(std::forward<Args>(args)...),
      std::make_index_sequence<sizeof...(Args)>());
}

int main()
{
  static_thread_pool pool{1};
  int result = invoke(pool.executor(), [](int i, int j){ return i + j; }, 20, 22);
  std::cout << "result is " << result << "\n";
}
