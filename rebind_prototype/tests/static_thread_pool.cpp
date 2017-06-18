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
void static_thread_pool_oneway_executor_compile_test(Executor ex1)
{
  static_thread_pool_executor_compile_test(ex1);

  static_assert(execution::is_oneway_executor_v<Executor>, "is_oneway_executor must evaluate true");

  const Executor& cex1 = ex1;

  cex1.execute([]{});

  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::never_blocking));
  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::possibly_blocking));
  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::always_blocking));
  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::is_continuation));
  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::is_not_continuation));
  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::is_work));
  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::is_not_work));
  static_thread_pool_oneway_executor_compile_test(cex1.require(execution::allocator(std::allocator<void>())));

  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::never_blocking));
  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::possibly_blocking));
  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::always_blocking));
  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::is_continuation));
  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::is_not_continuation));
  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::is_work));
  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::is_not_work));
  static_thread_pool_oneway_executor_compile_test(cex1.prefer(execution::allocator(std::allocator<void>())));
}

template<class Executor>
void static_thread_pool_twoway_executor_compile_test(Executor ex1)
{
  static_thread_pool_executor_compile_test(ex1);

  static_assert(execution::is_twoway_executor_v<Executor>, "is_twoway_executor must evaluate true");

  const Executor& cex1 = ex1;

  execution::executor_future_t<Executor, int> f1 = cex1.twoway_execute([]{ return 42; });
  int r1 = static_cast<const int&>(f1.get());
  (void)r1;

  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::never_blocking));
  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::possibly_blocking));
  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::always_blocking));
  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::is_continuation));
  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::is_not_continuation));
  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::is_work));
  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::is_not_work));
  static_thread_pool_twoway_executor_compile_test(cex1.require(execution::allocator(std::allocator<void>())));

  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::never_blocking));
  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::possibly_blocking));
  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::always_blocking));
  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::is_continuation));
  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::is_not_continuation));
  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::is_work));
  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::is_not_work));
  static_thread_pool_twoway_executor_compile_test(cex1.prefer(execution::allocator(std::allocator<void>())));
}

template<class Executor>
void static_thread_pool_bulk_oneway_executor_compile_test(Executor ex1)
{
  static_thread_pool_executor_compile_test(ex1);

  static_assert(execution::is_bulk_oneway_executor_v<Executor>, "is_bulk_oneway_executor must evaluate true");
  static_assert(std::is_same<execution::executor_shape_t<Executor>, std::size_t>::value, "shape type must be size_t");
  static_assert(std::is_same<execution::executor_index_t<Executor>, std::size_t>::value, "index type must be size_t");

  const Executor& cex1 = ex1;

  cex1.bulk_execute([](std::size_t, int&){}, 1, []{ return 42; });

  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::never_blocking));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::possibly_blocking));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::always_blocking));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::is_continuation));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::is_not_continuation));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::is_work));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::is_not_work));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.require(execution::allocator(std::allocator<void>())));

  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::never_blocking));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::possibly_blocking));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::always_blocking));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::is_continuation));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::is_not_continuation));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::is_work));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::is_not_work));
  static_thread_pool_bulk_oneway_executor_compile_test(cex1.prefer(execution::allocator(std::allocator<void>())));
}

template<class Executor>
void static_thread_pool_bulk_twoway_executor_compile_test(Executor ex1)
{
  static_thread_pool_executor_compile_test(ex1);

  static_assert(execution::is_bulk_twoway_executor_v<Executor>, "is_bulk_twoway_executor must evaluate true");
  static_assert(std::is_same<execution::executor_shape_t<Executor>, std::size_t>::value, "shape type must be size_t");
  static_assert(std::is_same<execution::executor_index_t<Executor>, std::size_t>::value, "index type must be size_t");

  const Executor& cex1 = ex1;

  execution::executor_future_t<Executor, void> f1 = cex1.bulk_twoway_execute([](std::size_t, int&){}, 1, []{}, []{ return 42; });
  f1.get();

  execution::executor_future_t<Executor, int> f2 = cex1.bulk_twoway_execute([](std::size_t, int&, int&){}, 1, []{ return 0; }, []{ return 42; });
  int r2 = static_cast<const int&>(f2.get());
  (void)r2;

  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::never_blocking));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::possibly_blocking));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::always_blocking));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::is_continuation));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::is_not_continuation));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::is_work));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::is_not_work));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.require(execution::allocator(std::allocator<void>())));

  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::never_blocking));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::possibly_blocking));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::always_blocking));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::is_continuation));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::is_not_continuation));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::is_work));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::is_not_work));
  static_thread_pool_bulk_twoway_executor_compile_test(cex1.prefer(execution::allocator(std::allocator<void>())));
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

  static_thread_pool_oneway_executor_compile_test(pool1.executor());
  static_thread_pool_oneway_executor_compile_test(pool1.executor().require(execution::oneway));
  static_thread_pool_oneway_executor_compile_test(pool1.executor().require(execution::twoway).require(execution::oneway));
  static_thread_pool_oneway_executor_compile_test(pool1.executor().require(execution::single));
  static_thread_pool_oneway_executor_compile_test(pool1.executor().require(execution::bulk).require(execution::single));
  static_thread_pool_twoway_executor_compile_test(pool1.executor());
  static_thread_pool_twoway_executor_compile_test(pool1.executor().require(execution::twoway));
  static_thread_pool_twoway_executor_compile_test(pool1.executor().require(execution::oneway).require(execution::twoway));
  static_thread_pool_bulk_oneway_executor_compile_test(pool1.executor());
  static_thread_pool_bulk_oneway_executor_compile_test(pool1.executor().require(execution::bulk));
  static_thread_pool_bulk_twoway_executor_compile_test(pool1.executor());
  static_thread_pool_bulk_twoway_executor_compile_test(pool1.executor().require(execution::bulk));
}

int main()
{
}
