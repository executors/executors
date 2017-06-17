#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template <class T>
class tracing_allocator
{
public:
  typedef T value_type;

  tracing_allocator() noexcept {}
  template <class U> tracing_allocator(const tracing_allocator<U>&) noexcept {}

  T* allocate(std::size_t n)
  {
    std::cout << "Allocating " << n << " of type " << typeid(T).name() << "\n";
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t n)
  {
    std::cout << "Deallocating " << n << " of type " << typeid(T).name() << "\n";
    ::operator delete(p, sizeof(T) * n);
  }
};

int main()
{
  static_thread_pool pool{1};
  auto ex = pool.executor().require(std::allocator_arg, tracing_allocator<char>{});
  ex.execute([]{ std::cout << "we made it\n"; });
  pool.wait();
}
