#include <experimental/thread_pool>
#include <iostream>
#include <memory>
#include <string>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template <class InnerExecutor>
class logging_executor
{
  std::shared_ptr<std::string> prefix_;
  InnerExecutor inner_ex_;

public:
  logging_executor(const std::string& prefix, const InnerExecutor& ex)
    : prefix_(std::make_shared<std::string>(prefix)), inner_ex_(ex) {}

  template <class... T> auto rebind(T&&... t) const &
    -> logging_executor<execution::rebind_member_result_t<InnerExecutor, T...>>
      { return { *prefix_, inner_ex_.rebind(std::forward<T>(t)...) }; }
  template <class... T> auto rebind(T&&... t) &&
    -> logging_executor<execution::rebind_member_result_t<InnerExecutor&&, T...>>
      { return { *prefix_, std::move(inner_ex_).rebind(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const logging_executor& a, const logging_executor& b) noexcept
  {
    return *a.prefix_ == *b.prefix_ && a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const logging_executor& a, const logging_executor& b) noexcept
  {
    return !(a == b);
  }

  template <class Function>
  auto operator()(Function f) const
  {
    return inner_ex_(
        [prefix = *prefix_, f = std::move(f)]() mutable
        {
          std::cout << prefix << ": " << "function begins\n";

          struct on_exit
          {
            std::string& prefix;
            ~on_exit() { std::cout << prefix << ": " << "function ends\n"; }
          } x{prefix};

          return f();
        });
  }
};

static_assert(execution::is_one_way_executor_v<
  logging_executor<static_thread_pool::executor_type>>,
    "one way executor requirements must be met");

int main()
{
  static_thread_pool pool{1};
  logging_executor<static_thread_pool::executor_type> ex1("LOG", pool.executor());
  ex1([]{ std::cout << "we made it\n"; });
  auto ex2 = ex1.rebind(execution::always_blocking);
  ex2([]{ std::cout << "we made it again\n"; });
  auto ex3 = ex2.rebind(execution::never_blocking).rebind(execution::is_continuation);
  ex3([]{ std::cout << "and again\n"; });
  auto ex4 = ex1.rebind(execution::two_way);
  std::future<int> f = ex4([]{ std::cout << "computing result\n"; return 42; });
  pool.wait();
  std::cout << "result is " << f.get() << "\n";
}
