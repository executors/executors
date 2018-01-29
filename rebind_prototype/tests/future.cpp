#include <experimental/future>

namespace execution = std::experimental::execution;
template<class R> using promise = std::experimental::executors_v1::promise<R>;
template<class R> using future = std::experimental::executors_v1::future<R>;
template<class R> using packaged_task = std::experimental::executors_v1::packaged_task<R>;

struct move_only
{
  move_only() = default;
  move_only(move_only&&) = default;
  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;
};

void promise_compile_test()
{
  promise<move_only> p1;
  promise<move_only> p2(std::allocator_arg, std::allocator<char>());
  promise<move_only> p3(std::move(p1));
  promise<move_only> p4 = std::move(p3);

  p1 = std::move(p4);
  p3.swap(p2);
  swap(p2, p3);

  future<move_only> f1(p1.get_future());

  p1.set_value(move_only());

  p1.set_exception(std::make_exception_ptr(std::runtime_error("fail")));
}

void promise_void_compile_test()
{
  promise<void> p1;
  promise<void> p2(std::allocator_arg, std::allocator<char>());
  promise<void> p3(std::move(p1));
  promise<void> p4 = std::move(p3);

  p1 = std::move(p4);
  p3.swap(p2);
  swap(p2, p3);

  future<void> f1(p1.get_future());

  p1.set_value();

  p1.set_exception(std::make_exception_ptr(std::runtime_error("fail")));
}

void future_compile_test()
{
  promise<move_only> p1;

  future<move_only> f1(p1.get_future());
  future<move_only> f2 = p1.get_future();
  future<move_only> f3(std::move(f2));
  future<move_only> f4 = std::move(f3);

  promise<future<move_only>> p2;
  future<move_only> f5(p2.get_future());

  f1 = std::move(f4);

  move_only r = f1.get();
  (void)r;

  bool valid = f1.valid();
  (void)valid;

  f1.wait();

  f1.wait_for(std::chrono::seconds(1));

  f1.wait_until(std::chrono::system_clock::now() + std::chrono::seconds(1));

  future<void> f6 = f1.then([](future<move_only>){});
  future<int> f7 = f1.then([](future<move_only>){ return 42; });
  future<move_only> f8 = f1.then([](future<move_only> f){ return f; });
}

void future_void_compile_test()
{
  promise<void> p1;

  future<void> f1(p1.get_future());
  future<void> f2 = p1.get_future();
  future<void> f3(std::move(f2));
  future<void> f4 = std::move(f3);

  promise<future<void>> p2;
  future<void> f5(p2.get_future());

  f1 = std::move(f4);

  f1.get();

  bool valid = f1.valid();
  (void)valid;

  f1.wait();

  f1.wait_for(std::chrono::seconds(1));

  f1.wait_until(std::chrono::system_clock::now() + std::chrono::seconds(1));

  future<void> f6 = f1.then([](future<void>){});
  future<int> f7 = f1.then([](future<void>){ return 42; });
  future<void> f8 = f1.then([](future<void> f){ return f; });
}

int main()
{
}
