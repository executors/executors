#include <experimental/execution>
#include <iostream>

namespace execution = std::experimental::execution;

class inline_executor
{
public:
  auto& context() const noexcept { return *this; }

  friend bool operator==(const inline_executor&, const inline_executor&) noexcept
  {
    return true;
  }

  friend bool operator!=(const inline_executor&, const inline_executor&) noexcept
  {
    return false;
  }

  template <class Function>
  void operator()(Function f) const noexcept
  {
    f();
  }
};

static_assert(execution::is_one_way_executor_v<inline_executor>, "one way executor requirements not met");

int main()
{
  inline_executor ex;
  ex([]{ std::cout << "we made it\n"; });
}
