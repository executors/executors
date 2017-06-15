#include <experimental/thread_pool>
#include <experimental/execution>
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace impl
{

static_thread_pool system_thread_pool{std::max(1u,std::thread::hardware_concurrency())};

class system_thread_pool_bulk_executor
{
  public:
    using bulk_forward_progress_guarantee = execution::executor_bulk_forward_progress_guarantee_t<static_thread_pool::executor_type>;
    using shape_type = execution::executor_shape_t<static_thread_pool::executor_type>;
    using index_type = execution::executor_index_t<static_thread_pool::executor_type>;

    template<class T>
    using future = execution::executor_future_t<static_thread_pool::executor_type,T>;

    auto& context() const noexcept { return system_thread_pool; }

    friend bool operator==(const system_thread_pool_bulk_executor&, const system_thread_pool_bulk_executor&) noexcept
    {
      return true;
    }

    friend bool operator!=(const system_thread_pool_bulk_executor&, const system_thread_pool_bulk_executor&) noexcept
    {
      return false;
    }

    template<class Function, class ResultFactory, class SharedFactory>
    auto bulk_async_execute(Function f, size_t n, ResultFactory rf, SharedFactory sf) const
    {
      return system_thread_pool.executor().bulk_async_execute(std::move(f), n, std::move(rf), std::move(sf));
    }

    template<class Function, class SharedFactory>
    auto bulk_execute(Function f, size_t n, SharedFactory sf) const
    {
      return system_thread_pool.executor().bulk_execute(std::move(f), n, std::move(sf));
    }
};

template<class BulkForwardProgressRequirement, class Executor>
class basic_execution_policy
{
  public:
    //static_assert(is_weaker_than<
    //                BulkForwardProgressRequirement,
    //                executor_bulk_forward_progress_guarantee_t<Executor>
    //              >::value,
    //              "basic_execution_policy: BulkForwardProgressRequirement cannot be satisfied by Executor's guarantee."
    //);

    using executor_type = Executor;
    using bulk_forward_progress_requirement = BulkForwardProgressRequirement;

    basic_execution_policy() = default;

    basic_execution_policy(const basic_execution_policy&) = default;

    basic_execution_policy(executor_type&& exec)
      : executor_(std::move(exec))
    {}

    basic_execution_policy(const executor_type& exec)
      : executor_(exec)
    {}

    template<class OtherExecutor
             //, class = typename std::enable_if<
             //  is_weaker_than<
             //    BulkForwardProgressRequirement,
             //    executor_bulk_forward_progress_guarantee_t<typename std::decay<OtherExecutor>::type>
             //  >::value
             //>::type
            >
    basic_execution_policy<BulkForwardProgressRequirement,OtherExecutor> on(OtherExecutor&& exec) const
    {
      return basic_execution_policy<BulkForwardProgressRequirement,OtherExecutor>(std::forward<OtherExecutor>(exec));
    }

    executor_type executor() const
    {
      return executor_;
    }

  private:
    executor_type executor_;
};

constexpr struct ignored {} ignore;

} // end impl

class parallel_policy : public impl::basic_execution_policy<execution::bulk_parallel_execution, impl::system_thread_pool_bulk_executor>
{
  using super_t = impl::basic_execution_policy<execution::bulk_parallel_execution, impl::system_thread_pool_bulk_executor>;

  public:
    using super_t::super_t;
};

constexpr parallel_policy par{};

template<class ExecutionPolicy, class RandomAccessIterator, class Function>
void for_each(ExecutionPolicy&& policy, RandomAccessIterator first, RandomAccessIterator last, Function f)
{
  auto n = last - first;

  auto two_way_bulk_exec = execution::rebind(execution::rebind(policy.executor(), execution::bulk), execution::two_way);

  two_way_bulk_exec.bulk_async_execute([=](size_t idx, impl::ignored&)
  {
    f(first[idx]);
  },
  n,
  []{},
  []{ return impl::ignore; }
  ).get();
}

int main()
{
  static_thread_pool pool{1};

  std::vector<int> vec(10);

  for_each(par.on(pool.executor()), vec.begin(), vec.end(), [](int& x)
  {
    x = 42;
  });

  assert(std::count(vec.begin(), vec.end(), 42) == static_cast<int>(vec.size()));

  std::cout << "OK" << std::endl;
}
