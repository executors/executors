#ifndef STD_EXPERIMENTAL_BITS_STATIC_THREAD_POOL_H
#define STD_EXPERIMENTAL_BITS_STATIC_THREAD_POOL_H

#include <condition_variable>
#include <cstddef>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <new>
#include <thread>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {

class static_thread_pool
{
  class single_base {};
  class bulk_base { using shape_type = std::size_t; };

  static single_base base_factory(execution::single_t) { return {}; }
  static bulk_base base_factory(execution::bulk_t) { return {}; }

  template<class, class T, class U> struct dependent_is_same : std::is_same<T, U> {};

  template<class Directionality, class Cardinality, class Blocking, class Continuation, class Work, class ProtoAllocator>
  class executor_impl : public decltype(static_thread_pool::base_factory(Cardinality{}))
  {
    friend class static_thread_pool;
    static_thread_pool* pool_;
    ProtoAllocator allocator_;

    executor_impl(static_thread_pool* p, const ProtoAllocator& a) noexcept
      : pool_(p), allocator_(a) { pool_->work_up(Work{}); }

  public:
    executor_impl(const executor_impl& other) noexcept : pool_(other.pool_) { pool_->work_up(Work{}); }
    ~executor_impl() { pool_->work_down(Work{}); }

    // Directionality.
    executor_impl<execution::one_way_t, Cardinality, Blocking, Continuation, Work, ProtoAllocator>
      rebind(execution::one_way_t) const { return {pool_, allocator_}; };
    executor_impl<execution::two_way_t, Cardinality, Blocking, Continuation, Work, ProtoAllocator>
      rebind(execution::two_way_t) const { return {pool_, allocator_}; };

    // Cardinality.
    executor_impl<Directionality, execution::single_t, Blocking, Continuation, Work, ProtoAllocator>
      rebind(execution::single_t) const { return {pool_, allocator_}; };
    executor_impl<Directionality, execution::bulk_t, Blocking, Continuation, Work, ProtoAllocator>
      rebind(execution::bulk_t) const { return {pool_, allocator_}; };

    // Blocking modes.
    executor_impl<Directionality, Cardinality, execution::never_blocking_t, Continuation, Work, ProtoAllocator>
      rebind(execution::never_blocking_t) const { return {pool_, allocator_}; };
    executor_impl<Directionality, Cardinality, execution::possibly_blocking_t, Continuation, Work, ProtoAllocator>
      rebind(execution::possibly_blocking_t) const { return {pool_, allocator_}; };
    executor_impl<Directionality, Cardinality, execution::always_blocking_t, Continuation, Work, ProtoAllocator>
      rebind(execution::always_blocking_t) const { return {pool_, allocator_}; };

    // Continuation hint.
    executor_impl<Directionality, Cardinality, Blocking, execution::is_continuation_t, Work, ProtoAllocator>
      rebind(execution::is_continuation_t) const { return {pool_, allocator_}; };
    executor_impl<Directionality, Cardinality, Blocking, execution::is_not_continuation_t, Work, ProtoAllocator>
      rebind(execution::is_not_continuation_t) const { return {pool_, allocator_}; };

    // Work tracking.
    executor_impl<Directionality, Cardinality, Blocking, Continuation, execution::is_work_t, ProtoAllocator>
      rebind(execution::is_work_t) const { return {pool_, allocator_}; };
    executor_impl<Directionality, Cardinality, Blocking, Continuation, execution::is_not_work_t, ProtoAllocator>
      rebind(execution::is_not_work_t) const { return {pool_, allocator_}; };

    // Allocator.
    template <class NewProtoAllocator>
    executor_impl<Directionality, Cardinality, Blocking, Continuation, execution::is_not_work_t, NewProtoAllocator>
      rebind(std::allocator_arg_t, const NewProtoAllocator& alloc) const { return {pool_, alloc}; };

    bool running_in_this_thread() const noexcept { return pool_->running_in_this_thread(); }

    static_thread_pool& context() const noexcept { return *pool_; }

    friend bool operator==(const executor_impl& a, const executor_impl& b) noexcept
    {
      return a.pool_ == b.pool_;
    }

    friend bool operator!=(const executor_impl& a, const executor_impl& b) noexcept
    {
      return a.pool_ != b.pool_;
    }

    template<class Function> auto operator()(Function f) const ->
      typename std::enable_if<
        dependent_is_same<Function, Directionality, execution::one_way_t>::value
          && dependent_is_same<Function, Cardinality, execution::single_t>::value
      >::type
    {
      pool_->execute(Directionality{}, Cardinality{}, Blocking{}, Continuation{}, allocator_, std::move(f));
    }

    template<class Function> auto operator()(Function f) const ->
      typename std::enable_if<
        dependent_is_same<Function, Directionality, execution::two_way_t>::value
          && dependent_is_same<Function, Cardinality, execution::single_t>::value,
        std::future<decltype(f())>
      >::type
    {
      return pool_->execute(Directionality{}, Cardinality{}, Blocking{}, Continuation{}, allocator_, std::move(f));
    }

    template<class Function, class SharedFactory> auto operator()(Function f, std::size_t n, SharedFactory sf) const ->
      typename std::enable_if<
        dependent_is_same<Function, Directionality, execution::one_way_t>::value
          && dependent_is_same<Function, Cardinality, execution::bulk_t>::value
      >::type
    {
      return pool_->execute(Directionality{}, Cardinality{}, Blocking{}, Continuation{}, allocator_, std::move(f), n, std::move(sf));
    }

#if 0 // Not yet implemented.
    template<class Function, class SharedFactory> auto operator()(Function f, std::size_t n, SharedFactory sf) const ->
      typename std::enable_if<
        dependent_is_same<Function, Directionality, execution::two_way_t>::value
          && dependent_is_same<Function, Cardinality, execution::bulk_t>::value
      >::type;
#endif
  };

public:
  using executor_type = executor_impl<execution::one_way_t, execution::single_t, execution::possibly_blocking_t,
      execution::is_not_continuation_t, execution::is_not_work_t, std::allocator<void>>;

  explicit static_thread_pool(std::size_t threads)
  {
    for (std::size_t i = 0; i < threads; ++i)
      threads_.emplace_back([this]{ attach(); });
  }

  static_thread_pool(const static_thread_pool&) = delete;
  static_thread_pool& operator=(const static_thread_pool&) = delete;

  ~static_thread_pool()
  {
    stop();
    wait();
  }

  executor_type executor() noexcept
  {
    return executor_type{this, std::allocator<void>{}};
  }

  void attach()
  {
    thread_private_state private_state{this};
    for (std::unique_lock<std::mutex> lock(mutex_);;)
    {
      condition_.wait(lock, [this]{ return stopped_ || work_ == 0 || head_; });
      if (stopped_ || (work_ == 0 && !head_)) return;
      func_base* func = head_.release();
      head_ = std::move(func->next_);
      tail_ = head_ ? tail_ : &head_;
      lock.unlock();
      func->call();
      lock.lock();
      if (private_state.head_)
      {
        *tail_ = std::move(private_state.head_);
        tail_ = private_state.tail_;
        private_state.tail_ = &private_state.head_;
        // TODO notify other threads if more than one in private queue
      }
    }
  }

  void stop()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    stopped_ = true;
    condition_.notify_all();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    std::list<std::thread> threads(std::move(threads_));
    if (!threads.empty())
    {
      --work_;
      condition_.notify_all();
      lock.unlock();
      for (auto& t : threads)
        t.join();
    }
  }

private:
  template<class Function>
  static void invoke(Function& f) noexcept // Exceptions mean std::terminate.
  {
    f();
  }

  struct func_base
  {
    struct deleter { void operator()(func_base* p) { p->destroy(); } };
    using pointer = std::unique_ptr<func_base, deleter>;

    virtual ~func_base() {}
    virtual void call() = 0;
    virtual void destroy() = 0;

    pointer next_;
  };

  template<class Function, class ProtoAllocator>
  struct func : func_base
  {
    explicit func(Function f, const ProtoAllocator& a) : function_(std::move(f)), allocator_(a) {}

    static func_base::pointer create(Function f, const ProtoAllocator& a)
		{
			typename std::allocator_traits<ProtoAllocator>::template rebind_alloc<func> allocator(a);
			func* raw_p = allocator.allocate(1);
			try
			{
        func* p = new (raw_p) func(std::move(f), a);
				return func_base::pointer(p);
			}
			catch (...)
			{
				allocator.deallocate(raw_p, 1);
				throw;
			}
		}

    virtual void call()
    {
      func_base::pointer fp(this);
      Function f(std::move(function_));
      fp.reset();
      static_thread_pool::invoke(f);
    }

    virtual void destroy()
    {
      func* p = this;
      p->~func();
      allocator_.deallocate(p, 1);
    }

    Function function_;
    typename std::allocator_traits<ProtoAllocator>::template rebind_alloc<func> allocator_;
  };

  struct thread_private_state
  {
    static_thread_pool* pool_;
    func_base::pointer head_;
    func_base::pointer* tail_{&head_};
    thread_private_state* prev_state_{instance()};

    explicit thread_private_state(static_thread_pool* p) : pool_(p) { instance() = this; }
    ~thread_private_state() { instance() = prev_state_; }

    static thread_private_state*& instance()
    {
      static thread_local thread_private_state* p;
      return p;
    }
  };

  bool running_in_this_thread() const noexcept
  {
    if (thread_private_state* private_state = thread_private_state::instance())
      if (private_state->pool_ == this)
        return true;
    return false;
  }

  template<class Blocking, class Continuation, class ProtoAllocator, class Function>
  void execute(execution::one_way_t, execution::single_t, Blocking, Continuation, const ProtoAllocator& alloc, Function f)
  {
    if (std::is_same<Blocking, execution::possibly_blocking_t>::value)
    {
      // Run immediately if already in the pool.
      if (thread_private_state* private_state = thread_private_state::instance())
      {
        if (private_state->pool_ == this)
        {
          static_thread_pool::invoke(f);
          return;
        }
      }
    }

    func_base::pointer fp(func<Function, ProtoAllocator>::create(std::move(f), alloc));

    if (std::is_same<Continuation, execution::is_continuation_t>::value)
    {
      // Push to thread-private queue if available.
      if (thread_private_state* private_state = thread_private_state::instance())
      {
        if (private_state->pool_ == this)
        {
          *private_state->tail_ = std::move(fp);
          private_state->tail_ = &(*private_state->tail_)->next_;
          return;
        }
      }
    }

    // Otherwise push to main queue.
    std::unique_lock<std::mutex> lock(mutex_);
    *tail_ = std::move(fp);
    tail_ = &(*tail_)->next_;
    condition_.notify_one();
  }

  template<class Continuation, class ProtoAllocator, class Function>
  void execute(execution::one_way_t, execution::single_t, execution::always_blocking_t, Continuation, const ProtoAllocator& alloc, Function f)
  {
    // Run immediately if already in the pool.
    if (thread_private_state* private_state = thread_private_state::instance())
    {
      if (private_state->pool_ == this)
      {
        static_thread_pool::invoke(f);
        return;
      }
    }

    // Otherwise, wrap the function with a promise that, when broken, will signal that the function is complete.
    std::promise<void> promise;
    std::future<void> future = promise.get_future();
    this->execute(execution::one_way, execution::single, execution::never_blocking, Continuation{}, alloc, [f = std::move(f), p = std::move(promise)]() mutable { f(); });
    future.wait();
  }

  template<class Blocking, class Continuation, class ProtoAllocator, class Function>
  auto execute(execution::two_way_t, execution::single_t, Blocking, Continuation, const ProtoAllocator& alloc, Function f) -> std::future<decltype(f())>
  {
    std::packaged_task<decltype(f())()> task(std::move(f));
    std::future<decltype(f())> future = task.get_future();
    this->execute(execution::one_way, execution::single, Blocking{}, Continuation{}, alloc, std::move(task));
    return future;
  }

  template<class Blocking, class Continuation, class ProtoAllocator, class Function, class SharedFactory>
  void execute(execution::one_way_t, execution::bulk_t, Blocking, Continuation,
      const ProtoAllocator& alloc, Function f, std::size_t n, SharedFactory sf)
  {
    auto wrapped_f = [f = std::move(f), ss = sf()](int n) mutable { f(n, ss); };
    auto shared_f = std::make_shared<decltype(wrapped_f)>(std::move(wrapped_f));

    func_base::pointer head;
    func_base::pointer* tail{&head};
    for (std::size_t i = 0; i < n; ++i)
    {
      auto indexed_f = [shared_f, i]() mutable { (*shared_f)(i); };
      *tail = func_base::pointer(func<decltype(indexed_f), ProtoAllocator>::create(std::move(indexed_f), alloc));
      tail = &(*tail)->next_;
    }

    if (std::is_same<Continuation, execution::is_continuation_t>::value)
    {
      // Push to thread-private queue if available.
      if (thread_private_state* private_state = thread_private_state::instance())
      {
        if (private_state->pool_ == this)
        {
          *private_state->tail_ = std::move(head);
          private_state->tail_ = tail;
          return;
        }
      }
    }

    // Otherwise push to main queue.
    std::unique_lock<std::mutex> lock(mutex_);
    *tail_ = std::move(head);
    tail_ = tail;
    condition_.notify_all();
  }

  template<class Continuation, class ProtoAllocator, class Function, class SharedFactory>
  void execute(execution::one_way_t, execution::bulk_t, execution::always_blocking_t, Continuation,
      const ProtoAllocator& alloc, Function f, std::size_t n, SharedFactory sf)
  {
    // Wrap the function with a promise that, when broken, will signal that the function is complete.
    std::promise<void> promise;
    std::future<void> future = promise.get_future();
    auto wrapped_f = [f = std::move(f), p = std::move(promise)](std::size_t n, auto& s) mutable { f(n, s); };
    this->execute(execution::one_way, execution::bulk, execution::never_blocking, Continuation{}, alloc, std::move(wrapped_f), n, std::move(sf));
    future.wait();
  }

  void work_up(execution::is_work_t) noexcept
  {
    std::unique_lock<std::mutex> lock(mutex_);
    ++work_;
  }

  void work_down(execution::is_work_t) noexcept
  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (--work_ == 0)
      condition_.notify_all();
  }

  void work_up(execution::is_not_work_t) noexcept {}
  void work_down(execution::is_not_work_t) noexcept {}

  std::mutex mutex_;
  std::condition_variable condition_;
  std::list<std::thread> threads_;
  func_base::pointer head_;
  func_base::pointer* tail_{&head_};
  bool stopped_{false};
  std::size_t work_{1};
};

inline bool operator==(const static_thread_pool& a, const static_thread_pool& b) noexcept
{
  return std::addressof(a) == std::addressof(b);
}

inline bool operator!=(const static_thread_pool& a, const static_thread_pool& b) noexcept
{
  return !(a == b);
}

} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_STATIC_THREAD_POOL_H
