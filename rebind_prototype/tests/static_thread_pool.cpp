#include <experimental/thread_pool>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template<class Executor>
void static_thread_pool_executor_compile_test(Executor ex1)
{
  static_assert(execution::is_executor_v<Executor>, "is_executor must evaluate true");

  const Executor& cex1 = ex1;

  static_assert(noexcept(Executor(ex1)), "copy constructor must not throw");
  static_assert(noexcept(Executor(cex1)), "copy constructor must not throw");
  static_assert(noexcept(Executor(std::move(ex1))), "move constructor must not throw");

  Executor ex2(cex1);
  Executor ex3(std::move(ex1));

  static_assert(noexcept(ex2 = cex1), "copy assignment must not throw");
  static_assert(noexcept(ex3 = std::move(ex1)), "move assignment must not throw");

  ex2 = cex1;
  ex3 = std::move(ex1);

  bool b1 = cex1.running_in_this_thread();
  (void)b1;

  static_thread_pool& pool = cex1.context();
  (void)pool;

  const Executor& cex2 = ex2;

  bool b2 = (cex1 == cex2);
  (void)b2;

  bool b3 = (cex1 != cex2);
  (void)b3;
}

template<class Executor>
void static_thread_pool_one_way_executor_compile_test(Executor ex1)
{
  static_thread_pool_executor_compile_test(ex1);

  static_assert(execution::is_one_way_executor_v<Executor>, "is_one_way_executor must evaluate true");
  static_assert(!execution::is_two_way_executor_v<Executor>, "is_two_way_executor must evaluate false");

  const Executor& cex1 = ex1;

  cex1([]{});

  static_thread_pool_one_way_executor_compile_test(cex1.rebind(execution::never_blocking));
  static_thread_pool_one_way_executor_compile_test(cex1.rebind(execution::possibly_blocking));
  static_thread_pool_one_way_executor_compile_test(cex1.rebind(execution::always_blocking));
  static_thread_pool_one_way_executor_compile_test(cex1.rebind(execution::is_continuation));
  static_thread_pool_one_way_executor_compile_test(cex1.rebind(execution::is_not_continuation));
  static_thread_pool_one_way_executor_compile_test(cex1.rebind(execution::is_work));
  static_thread_pool_one_way_executor_compile_test(cex1.rebind(execution::is_not_work));
  static_thread_pool_one_way_executor_compile_test(cex1.rebind(std::allocator_arg, std::allocator<void>()));
}

template<class Executor>
void static_thread_pool_two_way_executor_compile_test(Executor ex1)
{
  static_thread_pool_executor_compile_test(ex1);

  static_assert(execution::is_two_way_executor_v<Executor>, "is_two_way_executor must evaluate true");
  static_assert(!execution::is_one_way_executor_v<Executor>, "is_one_way_executor must evaluate false");

  const Executor& cex1 = ex1;

  execution::executor_future_t<Executor, int> f1 = cex1([]{ return 42; });
  int r1 = static_cast<const int&>(f1.get());
  (void)r1;

  static_thread_pool_two_way_executor_compile_test(cex1.rebind(execution::never_blocking));
  static_thread_pool_two_way_executor_compile_test(cex1.rebind(execution::possibly_blocking));
  static_thread_pool_two_way_executor_compile_test(cex1.rebind(execution::always_blocking));
  static_thread_pool_two_way_executor_compile_test(cex1.rebind(execution::is_continuation));
  static_thread_pool_two_way_executor_compile_test(cex1.rebind(execution::is_not_continuation));
  static_thread_pool_two_way_executor_compile_test(cex1.rebind(execution::is_work));
  static_thread_pool_two_way_executor_compile_test(cex1.rebind(execution::is_not_work));
  static_thread_pool_two_way_executor_compile_test(cex1.rebind(std::allocator_arg, std::allocator<void>()));
}

template<class Executor>
void static_thread_pool_bulk_one_way_executor_compile_test(Executor ex1)
{
  static_thread_pool_executor_compile_test(ex1);

  static_assert(execution::is_bulk_one_way_executor_v<Executor>, "is_bulk_one_way_executor must evaluate true");
  static_assert(!execution::is_one_way_executor_v<Executor>, "is_one_way_executor must evaluate false");
  static_assert(!execution::is_two_way_executor_v<Executor>, "is_two_way_executor must evaluate false");

  static_assert(std::is_same<execution::executor_shape_t<Executor>, std::size_t>::value, "shape type must be size_t");
  static_assert(std::is_same<execution::executor_index_t<Executor>, std::size_t>::value, "index type must be size_t");

  const Executor& cex1 = ex1;

  cex1([](std::size_t, int&){}, 1, []{ return 42; });

  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(execution::never_blocking));
  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(execution::possibly_blocking));
  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(execution::always_blocking));
  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(execution::is_continuation));
  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(execution::is_not_continuation));
  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(execution::is_work));
  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(execution::is_not_work));
  static_thread_pool_bulk_one_way_executor_compile_test(cex1.rebind(std::allocator_arg, std::allocator<void>()));
}

void static_thread_pool_compile_test()
{
  using executor_type = static_thread_pool::executor_type;

  static_thread_pool pool1(0);
  static_thread_pool pool2(static_cast<std::size_t>(0));

  pool1.attach();

  pool1.stop();

  pool1.wait();

  executor_type ex1(pool1.executor());

  const static_thread_pool& cpool1 = pool1;
  const static_thread_pool& cpool2 = pool1;

  bool b1 = (cpool1 == cpool2);
  (void)b1;

  bool b2 = (cpool1 != cpool2);
  (void)b2;

  static_thread_pool_one_way_executor_compile_test(pool1.executor());
  static_thread_pool_one_way_executor_compile_test(pool1.executor().rebind(execution::one_way));
  static_thread_pool_one_way_executor_compile_test(pool1.executor().rebind(execution::two_way).rebind(execution::one_way));
  static_thread_pool_one_way_executor_compile_test(pool1.executor().rebind(execution::single));
  static_thread_pool_one_way_executor_compile_test(pool1.executor().rebind(execution::bulk).rebind(execution::single));
  static_thread_pool_two_way_executor_compile_test(pool1.executor().rebind(execution::two_way));
  static_thread_pool_two_way_executor_compile_test(pool1.executor().rebind(execution::one_way).rebind(execution::two_way));
  static_thread_pool_bulk_one_way_executor_compile_test(pool1.executor().rebind(execution::bulk));
}

int main()
{
}
