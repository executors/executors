#ifndef STD_EXPERIMENTAL_BITS_EXECUTOR_H
#define STD_EXPERIMENTAL_BITS_EXECUTOR_H

#include <atomic>
#include <experimental/bits/bad_executor.h>
#include <experimental/future>
#include <memory>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

class executor
{
  template<class R, class... Args>
  struct single_use_func_base
  {
    virtual ~single_use_func_base() {}
    virtual R call(Args...) = 0;
  };

  template<class Function, class R, class... Args>
  struct single_use_func : single_use_func_base<R, Args...>
  {
    Function function_;

    explicit single_use_func(Function f) : function_(std::move(f)) {}

    virtual R call(Args... args)
    {
      std::unique_ptr<single_use_func> fp(this);
      Function f(std::move(function_));
      fp.reset();
      return f(std::forward<Args>(args)...);
    }
  };

  template<class R, class... Args>
  struct multi_use_func_base
  {
    virtual ~multi_use_func_base() {}
    virtual R call(Args...) = 0;
  };

  template<class Function, class R, class... Args>
  struct multi_use_func : multi_use_func_base<R, Args...>
  {
    Function function_;

    explicit multi_use_func(Function f) : function_(std::move(f)) {}

    virtual R call(Args... args)
    {
      return function_(std::forward<Args>(args)...);
    }
  };

  using oneway_func_base = single_use_func_base<void>;
  template<class Function> using oneway_func = single_use_func<Function, void>;

  using shared_factory_base = multi_use_func_base<std::shared_ptr<void>>;
  template<class SharedFactory> using shared_factory = multi_use_func<SharedFactory, std::shared_ptr<void>>;

  using bulk_func_base = multi_use_func_base<void, std::size_t, std::shared_ptr<void>&>;
  template<class Function> using bulk_func = multi_use_func<Function, void, std::size_t, std::shared_ptr<void>&>;

  using twoway_func_base = single_use_func_base<std::shared_ptr<void>>;
  template<class Function> using twoway_func = single_use_func<Function, std::shared_ptr<void>>;

  using twoway_then_func_base = single_use_func_base<void, std::shared_ptr<void>, std::exception_ptr>;
  template<class Function> using twoway_then_func = single_use_func<Function, void, std::shared_ptr<void>, std::exception_ptr>;

  struct impl_base
  {
    virtual ~impl_base() {}
    virtual impl_base* clone() const noexcept = 0;
    virtual void destroy() noexcept = 0;
    virtual void executor_execute(std::unique_ptr<oneway_func_base> f) = 0;
    virtual void executor_twoway_execute(std::unique_ptr<twoway_func_base> f, std::unique_ptr<twoway_then_func_base> then) = 0;
    virtual void executor_bulk_execute(std::unique_ptr<bulk_func_base> f, std::size_t n, std::shared_ptr<shared_factory_base> sf) = 0;
    virtual const type_info& executor_target_type() const = 0;
    virtual void* executor_target() = 0;
    virtual const void* executor_target() const = 0;
    virtual bool executor_equals(const impl_base* e) const noexcept = 0;
    virtual impl_base* executor_require(never_blocking_t) const = 0;
    virtual impl_base* executor_require(possibly_blocking_t) const = 0;
    virtual impl_base* executor_require(always_blocking_t) const = 0;
    virtual impl_base* executor_prefer(continuation_t) const = 0;
    virtual impl_base* executor_prefer(not_continuation_t) const = 0;
    virtual impl_base* executor_prefer(outstanding_work_t) const = 0;
    virtual impl_base* executor_prefer(not_outstanding_work_t) const = 0;
    virtual impl_base* executor_prefer(bulk_sequenced_execution_t) const = 0;
    virtual impl_base* executor_prefer(bulk_parallel_execution_t) const = 0;
    virtual impl_base* executor_prefer(bulk_unsequenced_execution_t) const = 0;
    virtual impl_base* executor_prefer(new_thread_execution_mapping_t) const = 0;
    virtual const type_info& context_target_type() const = 0;
    virtual const void* context_target() const = 0;
    virtual bool context_equals(const impl_base* e) const noexcept = 0;
  };

  template<class Executor>
  struct impl : impl_base
  {
    Executor executor_;
    std::atomic<std::size_t> ref_count_{1};

    explicit impl(Executor ex) : executor_(std::move(ex)) {}

    virtual impl_base* clone() const noexcept
    {
      impl* e = const_cast<impl*>(this);
      ++e->ref_count_;
      return e;
    }

    virtual void destroy() noexcept
    {
      if (--ref_count_ == 0)
        delete this;
    }

    virtual void executor_execute(std::unique_ptr<oneway_func_base> f)
    {
      executor_.execute([f = std::move(f)]() mutable { f.release()->call(); });
    }

    virtual void executor_twoway_execute(std::unique_ptr<twoway_func_base> f, std::unique_ptr<twoway_then_func_base> then)
    {
      executor_.twoway_execute(
          [f = std::move(f)]() mutable
          {
            return f.release()->call();
          })
      .then(
          [then = std::move(then)](auto fut) mutable
          {
            std::shared_ptr<void> result;
            std::exception_ptr excep;
            try { result = fut.get(); } catch (...) { excep = std::current_exception(); }
            then.release()->call(result, excep);
          });
    }

    virtual void executor_bulk_execute(std::unique_ptr<bulk_func_base> f, std::size_t n, std::shared_ptr<shared_factory_base> sf)
    {
      executor_.bulk_execute(
          [f = std::move(f)](std::size_t i, auto s) mutable { f->call(i, s); }, n,
          [sf = std::move(sf)]() mutable { return sf->call(); });
    }

    virtual const type_info& executor_target_type() const
    {
      return typeid(executor_);
    }

    virtual void* executor_target()
    {
      return &executor_;
    }

    virtual const void* executor_target() const
    {
      return &executor_;
    }

    virtual bool executor_equals(const impl_base* e) const noexcept
    {
      if (this == e)
        return true;
      if (executor_target_type() != e->executor_target_type())
        return false;
      return executor_ == *static_cast<const Executor*>(e->executor_target());
    }

    virtual impl_base* executor_require(never_blocking_t) const
    {
      return new impl<decltype(execution::require(executor_, never_blocking))>(execution::require(executor_, never_blocking));
    }

    virtual impl_base* executor_require(possibly_blocking_t) const
    {
      return new impl<decltype(execution::require(executor_, possibly_blocking))>(execution::require(executor_, possibly_blocking));
    }

    virtual impl_base* executor_require(always_blocking_t) const
    {
      return new impl<decltype(execution::require(executor_, always_blocking))>(execution::require(executor_, always_blocking));
    }

    virtual impl_base* executor_prefer(continuation_t) const
    {
      return new impl<decltype(execution::prefer(executor_, continuation))>(execution::prefer(executor_, continuation));
    }

    virtual impl_base* executor_prefer(not_continuation_t) const
    {
      return new impl<decltype(execution::prefer(executor_, not_continuation))>(execution::prefer(executor_, not_continuation));
    }

    virtual impl_base* executor_prefer(outstanding_work_t) const
    {
      return new impl<decltype(execution::prefer(executor_, outstanding_work))>(execution::prefer(executor_, outstanding_work));
    }

    virtual impl_base* executor_prefer(not_outstanding_work_t) const
    {
      return new impl<decltype(execution::prefer(executor_, not_outstanding_work))>(execution::prefer(executor_, not_outstanding_work));
    }

    virtual impl_base* executor_prefer(bulk_sequenced_execution_t) const
    {
      return new impl<decltype(execution::prefer(executor_, bulk_sequenced_execution))>(execution::prefer(executor_, bulk_sequenced_execution));
    }

    virtual impl_base* executor_prefer(bulk_parallel_execution_t) const
    {
      return new impl<decltype(execution::prefer(executor_, bulk_parallel_execution))>(execution::prefer(executor_, bulk_parallel_execution));
    }

    virtual impl_base* executor_prefer(bulk_unsequenced_execution_t) const
    {
      return new impl<decltype(execution::prefer(executor_, bulk_unsequenced_execution))>(execution::prefer(executor_, bulk_unsequenced_execution));
    }

    virtual impl_base* executor_prefer(new_thread_execution_mapping_t) const
    {
      return new impl<decltype(execution::prefer(executor_, new_thread_execution_mapping))>(execution::prefer(executor_, new_thread_execution_mapping));
    }

    virtual const type_info& context_target_type() const
    {
      return typeid(executor_.context());
    }

    virtual const void* context_target() const
    {
      return &executor_.context();
    }

    virtual bool context_equals(const impl_base* i) const noexcept
    {
      if (this == i)
        return true;
      if (context_target_type() != i->context_target_type())
        return false;
      return executor_.context() == *static_cast<const executor_context_t<Executor>*>(i->context_target());
    }
  };

public:
  class context_type
  {
    friend class executor;
    impl_base *impl_{nullptr};
    context_type() {};

  public:
    context_type(const context_type&) = delete;
    context_type& operator=(const context_type&) = delete;

    friend bool operator==(const context_type& a, const context_type& b) noexcept
    {
      if (!a.impl_ && !b.impl_)
        return true;
      if (a.impl_ && b.impl_)
        return a.impl_->context_equals(b.impl_);
      return false;
    }

    friend bool operator!=(const context_type& a, const context_type& b) noexcept
    {
      return !(a == b);
    }

    template<class Context>
    friend bool operator==(const context_type& a, const Context& b) noexcept
    {
      if (a.impl_ && a.impl_->context_target_type() == typeid(Context))
        return *static_cast<const Context*>(a.impl_->context_target()) == b;
      return false;
    }

    template<class Context>
    friend bool operator!=(const context_type& a, const Context& b) noexcept
    {
      return !(a == b);
    }

    template<class Context>
    friend bool operator==(const Context& a, const context_type& b) noexcept
    {
      return b == a;
    }

    template<class Context>
    friend bool operator!=(const Context& a, const context_type& b) noexcept
    {
      return !(b == a);
    }
  };

  // construct / copy / destroy:

  executor() noexcept {}
  executor(std::nullptr_t) noexcept {}

  executor(const executor& e) noexcept
  {
    context_.impl_ = e.context_.impl_ ? e.context_.impl_->clone() : nullptr;
  }

  executor(executor&& e) noexcept
  {
    context_.impl_ = e.context_.impl_;
    e.context_.impl_ = nullptr;
  }

  template<class Executor> executor(Executor e)
  {
    auto e2 = execution::require(std::move(e), execution::single, execution::bulk, execution::oneway, execution::twoway);
    context_.impl_ = new impl<decltype(e2)>(std::move(e2));
  }

  executor& operator=(const executor& e) noexcept
  {
    if (context_.impl_) context_.impl_->destroy();
    context_.impl_ = e.context_.impl_ ? e.context_.impl_->clone() : nullptr;
    return *this;
  }

  executor& operator=(executor&& e) noexcept
  {
    if (this != &e)
    {
      if (context_.impl_) context_.impl_->destroy();
      context_.impl_ = e.context_.impl_;
      e.context_.impl_ = nullptr;
    }
    return *this;
  }

  executor& operator=(nullptr_t) noexcept
  {
    if (context_.impl_) context_.impl_->destroy();
    context_.impl_ = nullptr;
    return *this;
  }

  template<class Executor> executor& operator=(Executor e)
  {
    return operator=(executor(std::move(e)));
  }

  ~executor()
  {
    if (context_.impl_) context_.impl_->destroy();
  }

  // polymorphic executor modifiers:

  void swap(executor& other) noexcept
  {
    std::swap(context_.impl_, other.context_.impl_);
  }

  template<class Executor> void assign(Executor e)
  {
    operator=(executor(std::move(e)));
  }

  // executor operations:

  executor require(oneway_t) const { return *this; }
  executor require(twoway_t) const { return *this; }
  executor require(single_t) const { return *this; }
  executor require(bulk_t) const { return *this; }
  executor require(thread_execution_mapping_t) const { return *this; }

  executor require(never_blocking_t) const { return context_.impl_ ? context_.impl_->executor_require(never_blocking) : context_.impl_->clone(); }
  executor require(possibly_blocking_t) const { return context_.impl_ ? context_.impl_->executor_require(possibly_blocking) : context_.impl_->clone(); }
  executor require(always_blocking_t) const { return context_.impl_ ? context_.impl_->executor_require(always_blocking) : context_.impl_->clone(); }

  friend executor prefer(const executor& e, continuation_t) { return e.get_impl() ? e.get_impl()->executor_prefer(continuation) : e.get_impl()->clone(); }
  friend executor prefer(const executor& e, not_continuation_t) { return e.get_impl() ? e.get_impl()->executor_prefer(not_continuation) : e.get_impl()->clone(); }
  friend executor prefer(const executor& e, outstanding_work_t) { return e.get_impl() ? e.get_impl()->executor_prefer(outstanding_work) : e.get_impl()->clone(); }
  friend executor prefer(const executor& e, not_outstanding_work_t) { return e.get_impl() ? e.get_impl()->executor_prefer(not_outstanding_work) : e.get_impl()->clone(); }
  friend executor prefer(const executor& e, bulk_sequenced_execution_t) { return e.get_impl() ? e.get_impl()->executor_prefer(bulk_sequenced_execution) : e.get_impl()->clone(); }
  friend executor prefer(const executor& e, bulk_parallel_execution_t) { return e.get_impl() ? e.get_impl()->executor_prefer(bulk_parallel_execution) : e.get_impl()->clone(); }
  friend executor prefer(const executor& e, bulk_unsequenced_execution_t) { return e.get_impl() ? e.get_impl()->executor_prefer(bulk_unsequenced_execution) : e.get_impl()->clone(); }
  friend executor prefer(const executor& e, new_thread_execution_mapping_t) { return e.get_impl() ? e.get_impl()->executor_prefer(new_thread_execution_mapping) : e.get_impl()->clone(); }
  
  const context_type& context() const noexcept
  {
    return context_;
  }

  template<class Function> void execute(Function f) const
  {
    std::unique_ptr<oneway_func_base> fp(new oneway_func<Function>(std::move(f)));
    context_.impl_ ? context_.impl_->executor_execute(std::move(fp)) : throw bad_executor();
  }

  template<class Function> auto twoway_execute(Function f) const
    -> typename std::enable_if<std::is_same<decltype(f()), void>::value, future<void>>::type
  {
    promise<void> prom;
    future<void> fut(prom.get_future());

    auto f_wrap = [f = std::move(f)]() mutable
    {
      f();
      return std::shared_ptr<void>();
    };

    auto then = [prom = std::move(prom)](std::shared_ptr<void>, std::exception_ptr excep) mutable
    {
      if (excep)
        prom.set_exception(excep);
      else
        prom.set_value();
    };

    std::unique_ptr<twoway_func_base> fp(new twoway_func<decltype(f_wrap)>(std::move(f_wrap)));
    std::unique_ptr<twoway_then_func_base> tp(new twoway_then_func<decltype(then)>(std::move(then)));
    context_.impl_ ? context_.impl_->executor_twoway_execute(std::move(fp), std::move(tp)) : throw bad_executor();

    return fut;
  }

  template<class Function> auto twoway_execute(Function f) const
    -> typename std::enable_if<!std::is_same<decltype(f()), void>::value, future<decltype(f())>>::type
  {
    promise<decltype(f())> prom;
    future<decltype(f())> fut(prom.get_future());

    auto f_wrap = [f = std::move(f)]() mutable
    {
      return std::make_shared<decltype(f())>(f());
    };

    auto then = [prom = std::move(prom)](std::shared_ptr<void> result, std::exception_ptr excep) mutable
    {
      if (excep)
        prom.set_exception(excep);
      else
        prom.set_value(std::move(*std::static_pointer_cast<decltype(f())>(result)));
    };

    std::unique_ptr<twoway_func_base> fp(new twoway_func<decltype(f_wrap)>(std::move(f_wrap)));
    std::unique_ptr<twoway_then_func_base> tp(new twoway_then_func<decltype(then)>(std::move(then)));
    context_.impl_ ? context_.impl_->executor_twoway_execute(std::move(fp), std::move(tp)) : throw bad_executor();

    return fut;
  }

  template<class Function, class SharedFactory> void bulk_execute(Function f, std::size_t n, SharedFactory sf) const
  {
    auto f_wrap = [f = std::move(f)](std::size_t i, std::shared_ptr<void>& ss) mutable
    {
      f(i, *std::static_pointer_cast<decltype(sf())>(ss));
    };

    auto sf_wrap = [sf = std::move(sf)]() mutable
    {
      return std::make_shared<decltype(sf())>(sf());
    };

    std::unique_ptr<bulk_func_base> fp(new bulk_func<decltype(f_wrap)>(std::move(f_wrap)));
    std::shared_ptr<shared_factory_base> sfp(new shared_factory<decltype(sf_wrap)>(std::move(sf_wrap)));
    context_.impl_ ? context_.impl_->executor_bulk_execute(std::move(fp), n, std::move(sfp)) : throw bad_executor();
  }

#if 0 // TODO implement bulk two-way support.
  template<class Function, class ResultFactory, class SharedFactory>
    auto bulk_twoway_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const -> future<decltype(rf())>;
#endif

  // polymorphic executor capacity:

  explicit operator bool() const noexcept
  {
    return !!context_.impl_;
  }

  // polymorphic executor target access:

  const type_info& target_type() const noexcept
  {
    return context_.impl_ ? context_.impl_->executor_target_type() : typeid(void);
  }

  template<class Executor> Executor* target() noexcept
  {
    return context_.impl_ ? static_cast<Executor*>(context_.impl_->executor_target()) : nullptr;
  }

  template<class Executor> const Executor* target() const noexcept
  {
    return context_.impl_ ? static_cast<Executor*>(context_.impl_->executor_target()) : nullptr;
  }

  // polymorphic executor comparisons:

  friend bool operator==(const executor& a, const executor& b) noexcept
  {
    if (!a.get_impl() && !b.get_impl())
      return true;
    if (a.get_impl() && b.get_impl())
      return a.get_impl()->executor_equals(b.get_impl());
    return false;
  }

  friend bool operator==(const executor& e, nullptr_t) noexcept
  {
    return !e;
  }

  friend bool operator==(nullptr_t, const executor& e) noexcept
  {
    return !e;
  }

  friend bool operator!=(const executor& a, const executor& b) noexcept
  {
    return !(a == b);
  }

  friend bool operator!=(const executor& e, nullptr_t) noexcept
  {
    return !!e;
  }

  friend bool operator!=(nullptr_t, const executor& e) noexcept
  {
    return !!e;
  }

private:
  executor(impl_base* i) noexcept { context_.impl_ = i; }
  context_type context_;
  const impl_base* get_impl() const { return context_.impl_; }
};

// executor specialized algorithms:

inline void swap(executor& a, executor& b) noexcept
{
  a.swap(b);
}

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_EXECUTOR_H
