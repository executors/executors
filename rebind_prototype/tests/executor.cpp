#include <experimental/execution>
#include <experimental/thread_pool>

namespace execution = std::experimental::execution;
using execution::executor;
using std::experimental::static_thread_pool;
using std::experimental::concurrency_v2::future;

void executor_compile_test()
{
  static_assert(execution::is_executor_v<executor>, "is_executor must evaluate true");
  static_assert(execution::is_oneway_executor_v<executor>, "is_oneway_executor must evaluate true");
  static_assert(execution::is_twoway_executor_v<executor>, "is_twoway_executor must evaluate true");

  static_thread_pool pool(0);

  static_assert(noexcept(executor()), "default constructor must not throw");
  static_assert(noexcept(executor(nullptr)), "nullptr constructor must not throw");

  executor ex1;
  executor ex2(nullptr);

  const executor& cex1 = ex1;
  const executor& cex2 = ex2;

  static_assert(noexcept(executor(cex1)), "copy constructor must not throw");
  static_assert(noexcept(executor(std::move(ex1))), "move constructor must not throw");

  executor ex3(ex1);
  executor ex4(std::move(ex1));

  executor ex5(pool.executor());

  static_assert(noexcept(ex2 = cex1), "copy assignment must not throw");
  static_assert(noexcept(ex3 = std::move(ex1)), "move assignment must not throw");
  static_assert(noexcept(ex3 = nullptr), "nullptr assignment must not throw");

  ex2 = ex1;
  ex3 = std::move(ex1);
  ex4 = nullptr;
  ex5 = pool.executor();

  static_assert(noexcept(ex1.swap(ex2)), "swap must not throw");

  ex1.swap(ex2);

  ex1.assign(pool.executor());

  ex1 = cex1.require(execution::oneway);
  ex1 = cex1.require(execution::twoway);
  ex1 = cex1.require(execution::single);
  ex1 = cex1.require(execution::bulk);
  ex1 = cex1.require(execution::thread_execution_mapping);
  ex1 = cex1.require(execution::never_blocking);
  ex1 = cex1.require(execution::possibly_blocking);
  ex1 = cex1.require(execution::always_blocking);

  ex1 = execution::prefer(cex1, execution::thread_execution_mapping);
  ex1 = execution::prefer(cex1, execution::never_blocking);
  ex1 = execution::prefer(cex1, execution::possibly_blocking);
  ex1 = execution::prefer(cex1, execution::always_blocking);
  ex1 = execution::prefer(cex1, execution::continuation);
  ex1 = execution::prefer(cex1, execution::not_continuation);
  ex1 = execution::prefer(cex1, execution::outstanding_work);
  ex1 = execution::prefer(cex1, execution::not_outstanding_work);
  ex1 = execution::prefer(cex1, execution::bulk_sequenced_execution);
  ex1 = execution::prefer(cex1, execution::bulk_parallel_execution);
  ex1 = execution::prefer(cex1, execution::bulk_unsequenced_execution);
  ex1 = execution::prefer(cex1, execution::new_thread_execution_mapping);

  auto& context = execution::query(cex1, execution::context);
  (void)context;

  cex1.execute([]{});

  std::experimental::concurrency_v2::future<int> f1 = cex1.twoway_execute([]{ return 42; });
  (void)f1;

  std::experimental::concurrency_v2::future<void> f2 = cex1.twoway_execute([]{});
  (void)f2;

  cex1.bulk_execute([](std::size_t, int&){}, 1, []{ return 42; });

  bool b1 = static_cast<bool>(ex1);
  (void)b1;

  const std::type_info& target_type = cex1.target_type();
  (void)target_type;

  static_thread_pool::executor_type* ex6 = ex1.target<static_thread_pool::executor_type>();
  (void)ex6;

  const static_thread_pool::executor_type* cex6 = ex1.target<static_thread_pool::executor_type>();
  (void)cex6;

  bool b2 = (cex1 == cex2);
  (void)b2;

  bool b3 = (cex1 != cex2);
  (void)b3;

  bool b4 = (cex1 == nullptr);
  (void)b4;

  bool b5 = (cex1 != nullptr);
  (void)b5;

  bool b6 = (nullptr == cex2);
  (void)b6;

  bool b7 = (nullptr != cex2);
  (void)b7;

  swap(ex1, ex2);
}

int main()
{
}
