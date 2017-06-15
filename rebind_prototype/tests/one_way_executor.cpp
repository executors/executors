#include <experimental/execution>
#include <experimental/thread_pool>

namespace execution = std::experimental::execution;
using execution::one_way_executor;
using std::experimental::static_thread_pool;

void one_way_executor_compile_test()
{
  static_assert(execution::is_executor_v<one_way_executor>, "is_executor must evaluate true");
  static_assert(execution::is_one_way_executor_v<one_way_executor>, "is_one_way_executor must evaluate true");
  static_assert(!execution::is_two_way_executor_v<one_way_executor>, "is_two_way_executor must evaluate false");

  static_thread_pool pool(0);

  using context_type = one_way_executor::context_type;

  static_assert(noexcept(one_way_executor()), "default constructor must not throw");
  static_assert(noexcept(one_way_executor(nullptr)), "nullptr constructor must not throw");

  one_way_executor ex1;
  one_way_executor ex2(nullptr);

  const one_way_executor& cex1 = ex1;
  const one_way_executor& cex2 = ex2;

  static_assert(noexcept(one_way_executor(cex1)), "copy constructor must not throw");
  static_assert(noexcept(one_way_executor(std::move(ex1))), "move constructor must not throw");

  one_way_executor ex3(ex1);
  one_way_executor ex4(std::move(ex1));

  one_way_executor ex5(pool.executor());

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

  ex1 = cex1.rebind(execution::never_blocking);
  ex1 = cex1.rebind(execution::possibly_blocking);
  ex1 = cex1.rebind(execution::always_blocking);
  ex1 = cex1.rebind(execution::is_continuation);
  ex1 = cex1.rebind(execution::is_not_continuation);
  ex1 = cex1.rebind(execution::is_work);
  ex1 = cex1.rebind(execution::is_not_work);

  const context_type& context = cex1.context();
  (void)context;

  cex1.execute([]{});

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
