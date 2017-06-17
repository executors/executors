#include <chrono>
#include <experimental/thread_pool>
#include <iostream>
#include <mutex>
#include <string>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

struct strand_state
{
  std::mutex mutex_;
  std::list<std::function<void()>> queue_;
  bool locked_{false};
  std::thread::id owning_thread_;
};

template <class Executor, class Blocking = execution::possibly_blocking_t>
class strand
{
  template <class, class> friend class strand;

  std::shared_ptr<strand_state> state_;
  Executor ex_;

  strand(std::shared_ptr<strand_state> s, Executor ex)
    : state_(std::move(s)), ex_(std::move(ex))
  {
  }

  void run_first_item()
  {
    // Dequeue first item.
    std::unique_lock<std::mutex> lock(state_->mutex_);
    std::function<void()> f = std::move(state_->queue_.front());
    state_->queue_.pop_front();
    state_->owning_thread_ = std::this_thread::get_id();
    lock.unlock();

    // Execute the item.
    std::cout << "begin strand\n";
    f();
    std::cout << "end strand\n";

    // Check if there are any more items.
    lock.lock();
    state_->owning_thread_ = std::thread::id{};
    if (state_->queue_.empty())
    {
      state_->locked_ = false;
      return;
    }
    lock.unlock();

    // Need to reschedule the strand to run the queued items.
    Executor ex(ex_);
    ex.execute([s = std::move(*this)]() mutable
        {
          s.run_first_item();
        });
  }

public:
  static_assert(execution::is_oneway_executor_v<Executor>, "strand requires a one way executor");

  explicit strand(Executor ex)
    : state_(std::make_shared<strand_state>()), ex_(std::move(ex))
  {
  }

  template <class... T> auto require(T&&... t) const
    -> strand<execution::require_member_result_t<Executor, T...>, Blocking>
      { return { state_, ex_.require(std::forward<T>(t)...) }; }

  auto require(execution::never_blocking_t) const
    -> strand<execution::require_member_result_t<Executor, execution::never_blocking_t>, execution::never_blocking_t>
  {
    return {state_, ex_.require(execution::never_blocking)};
  };

  auto require(execution::possibly_blocking_t) const
    -> strand<execution::require_member_result_t<Executor, execution::possibly_blocking_t>, execution::never_blocking_t>
  {
    return {state_, ex_.require(execution::possibly_blocking)};
  };

  void require(execution::always_blocking_t) const = delete;

  template <class... T> auto prefer(T&&... t) const
    -> strand<execution::prefer_member_result_t<Executor, T...>, Blocking>
      { return { state_, ex_.prefer(std::forward<T>(t)...) }; }

  auto prefer(execution::never_blocking_t) const
    -> strand<execution::prefer_member_result_t<Executor, execution::never_blocking_t>, execution::never_blocking_t>
  {
    return {state_, ex_.prefer(execution::never_blocking)};
  };

  auto prefer(execution::possibly_blocking_t) const
    -> strand<execution::prefer_member_result_t<Executor, execution::possibly_blocking_t>, execution::never_blocking_t>
  {
    return {state_, ex_.prefer(execution::possibly_blocking)};
  };

  strand prefer(execution::always_blocking_t) const
  {
    return *this;
  }

  auto& context() const
  {
    return ex_.context();
  }

  friend bool operator==(const strand& a, const strand& b) noexcept
  {
    return a.state_ == b.state_;
  }

  friend bool operator!=(const strand& a, const strand& b) noexcept
  {
    return a.state_ != b.state_;
  }

  template <class Function>
  void execute(Function f) const
  {
    std::unique_lock<std::mutex> lock(state_->mutex_);

    // Determine if we are already inside the strand.
    bool in_strand = (state_->owning_thread_ == std::this_thread::get_id());

    // Execute immediately if possible.
    if (std::is_same<Blocking, execution::possibly_blocking_t>::value && in_strand)
    {
      lock.unlock();
      f();
      return;
    }

    // Try to grab the strand "lock".
    state_->queue_.emplace_back(std::move(f));
    if (state_->locked_) return; // Someone else already holds the "lock".
    state_->locked_ = true;
    lock.unlock();

    // Need to schedule the strand to run the queued items.
    ex_.execute([s = this->require(execution::never_blocking).require(execution::is_continuation)]() mutable
        {
          s.run_first_item();
        });
  }
};

static_assert(execution::is_oneway_executor_v<
  strand<static_thread_pool::executor_type>>,
    "one way executor requirements must be met");

struct foo
{
  decltype(std::declval<strand<static_thread_pool::executor_type>>().require(execution::never_blocking)) strand_;
  int count_{0};

  explicit foo(const strand<static_thread_pool::executor_type>& s)
    : strand_(s.require(execution::never_blocking))
  {
  }

  void operator()()
  {
    if (count_ < 10)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "count is " << count_ << "\n";
      strand_.require(execution::possibly_blocking).execute([count = count_]{ std::cout << "nested count is " << count << "\n"; });
      ++count_;
      strand_.execute(*this);
    }
  }
};

int main()
{
  static_thread_pool pool{2};
  strand<static_thread_pool::executor_type> s1(pool.executor());
  s1.require(execution::never_blocking).execute(foo{s1});
  s1.require(execution::possibly_blocking).execute([]{ std::cout << "After 0, before 1\n"; });
  pool.wait();
}
