#include <chrono>
#include <experimental/thread_pool>
#include <iostream>
#include <thread>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

// An operation that doubles a value asynchronously.
template <class TaskExecutor, class CompletionExecutor, class CompletionHandler>
void my_async_operation_1(const TaskExecutor& tex, int n,
    const CompletionExecutor& cex, CompletionHandler h)
{
  if (n == 0)
  {
    // Nothing to do. Operation finishes immediately.
    // Specify non-blocking to prevent stack overflow.
    cex.rebind(execution::never_blocking).execute(
        [h = std::move(h), n]() mutable
        {
          h(n);
        });
  }
  else
  {
    // Simulate an asynchronous operation.
    tex.rebind(execution::never_blocking).execute(
        [n, cex = cex.rebind(execution::is_work), h = std::move(h)]() mutable
        {
          int result = n * 2;
          std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate long running work.
          cex.rebind(execution::possibly_blocking).execute(
              [h = std::move(h), result]() mutable
              {
                h(result);
              });
        });
  }
}

int main()
{
  static_thread_pool task_pool{1};
  static_thread_pool completion_pool{1};
  my_async_operation_1(task_pool.executor(), 21, completion_pool.executor(),
      [](int n){ std::cout << "the answer is " << n << "\n"; });
  completion_pool.wait();
}
