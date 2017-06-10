#ifndef STD_EXPERIMENTAL_BITS_ONE_WAY_EXECUTOR_H
#define STD_EXPERIMENTAL_BITS_ONE_WAY_EXECUTOR_H

#include <atomic>
#include <experimental/bits/bad_executor.h>
#include <memory>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

class one_way_executor
{
  struct func_base
  {
    virtual ~func_base() {}
    virtual void call() = 0;
  };

  template<class Function>
  struct func : func_base
  {
    Function function_;

    explicit func(Function f) : function_(std::move(f)) {}

    virtual void call()
    {
      std::unique_ptr<func> fp(this);
      Function f(std::move(function_));
      fp.reset();
      f();
    }
  };

  struct impl_base
  {
    virtual ~impl_base() {}
    virtual impl_base* clone() const noexcept = 0;
    virtual void destroy() noexcept = 0;
    virtual void executor_call(std::unique_ptr<func_base> f) = 0;
    virtual const type_info& executor_target_type() const = 0;
    virtual void* executor_target() = 0;
    virtual const void* executor_target() const = 0;
    virtual bool executor_equals(const impl_base* e) const noexcept = 0;
    virtual impl_base* executor_rebind(never_blocking_t) const = 0;
    virtual impl_base* executor_rebind(possibly_blocking_t) const = 0;
    virtual impl_base* executor_rebind(always_blocking_t) const = 0;
    virtual impl_base* executor_rebind(is_continuation_t) const = 0;
    virtual impl_base* executor_rebind(is_not_continuation_t) const = 0;
    virtual impl_base* executor_rebind(is_work_t) const = 0;
    virtual impl_base* executor_rebind(is_not_work_t) const = 0;
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

    virtual void executor_call(std::unique_ptr<func_base> f)
    {
      executor_([f = std::move(f)]() mutable { f.release()->call(); });
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

    virtual impl_base* executor_rebind(never_blocking_t) const
    {
      return new impl<decltype(execution::rebind(executor_, never_blocking))>(execution::rebind(executor_, never_blocking));
    }

    virtual impl_base* executor_rebind(possibly_blocking_t) const
    {
      return new impl<decltype(execution::rebind(executor_, possibly_blocking))>(execution::rebind(executor_, possibly_blocking));
    }

    virtual impl_base* executor_rebind(always_blocking_t) const
    {
      return new impl<decltype(execution::rebind(executor_, always_blocking))>(execution::rebind(executor_, always_blocking));
    }

    virtual impl_base* executor_rebind(is_continuation_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_continuation))>(execution::rebind(executor_, is_continuation));
    }

    virtual impl_base* executor_rebind(is_not_continuation_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_not_continuation))>(execution::rebind(executor_, is_not_continuation));
    }

    virtual impl_base* executor_rebind(is_work_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_work))>(execution::rebind(executor_, is_work));
    }

    virtual impl_base* executor_rebind(is_not_work_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_not_work))>(execution::rebind(executor_, is_not_work));
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
    friend class one_way_executor;
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

  one_way_executor() noexcept {}
  one_way_executor(std::nullptr_t) noexcept {}

  one_way_executor(const one_way_executor& e) noexcept
  {
    context_.impl_ = e.context_.impl_ ? e.context_.impl_->clone() : nullptr;
  }

  one_way_executor(one_way_executor&& e) noexcept
  {
    context_.impl_ = e.context_.impl_;
    e.context_.impl_ = nullptr;
  }

  template<class Executor> one_way_executor(Executor e)
  {
    context_.impl_ = new impl<Executor>(std::move(e));
  }

  one_way_executor& operator=(const one_way_executor& e) noexcept
  {
    if (context_.impl_) context_.impl_->destroy();
    context_.impl_ = e.context_.impl_ ? e.context_.impl_->clone() : nullptr;
    return *this;
  }

  one_way_executor& operator=(one_way_executor&& e) noexcept
  {
    if (this != &e)
    {
      if (context_.impl_) context_.impl_->destroy();
      context_.impl_ = e.context_.impl_;
      e.context_.impl_ = nullptr;
    }
    return *this;
  }

  one_way_executor& operator=(nullptr_t) noexcept
  {
    if (context_.impl_) context_.impl_->destroy();
    context_.impl_ = nullptr;
    return *this;
  }

  template<class Executor> one_way_executor& operator=(Executor e)
  {
    return operator=(one_way_executor(std::move(e)));
  }

  ~one_way_executor()
  {
    if (context_.impl_) context_.impl_->destroy();
  }

  // polymorphic executor modifiers:

  void swap(one_way_executor& other) noexcept
  {
    std::swap(context_.impl_, other.context_.impl_);
  }

  template<class Executor> void assign(Executor e)
  {
    operator=(one_way_executor(std::move(e)));
  }

  // one_way_executor operations:

  one_way_executor rebind(never_blocking_t) const { return context_.impl_ ? context_.impl_->executor_rebind(never_blocking) : context_.impl_->clone(); }
  one_way_executor rebind(possibly_blocking_t) const { return context_.impl_ ? context_.impl_->executor_rebind(possibly_blocking) : context_.impl_->clone(); }
  one_way_executor rebind(always_blocking_t) const { return context_.impl_ ? context_.impl_->executor_rebind(always_blocking) : context_.impl_->clone(); }
  one_way_executor rebind(is_continuation_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_continuation) : context_.impl_->clone(); }
  one_way_executor rebind(is_not_continuation_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_not_continuation) : context_.impl_->clone(); }
  one_way_executor rebind(is_work_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_work) : context_.impl_->clone(); }
  one_way_executor rebind(is_not_work_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_not_work) : context_.impl_->clone(); }
  
  const context_type& context() const noexcept
  {
    return context_;
  }

  template<class Function> void operator()(Function f) const
  {
    std::unique_ptr<func_base> fp(new func<Function>(std::move(f)));
    context_.impl_ ? context_.impl_->executor_call(std::move(fp)) : throw bad_executor();
  }

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

  friend bool operator==(const one_way_executor& a, const one_way_executor& b) noexcept
  {
    if (!a.get_impl() && !b.get_impl())
      return true;
    if (a.get_impl() && b.get_impl())
      return a.get_impl()->executor_equals(b.get_impl());
    return false;
  }

  friend bool operator==(const one_way_executor& e, nullptr_t) noexcept
  {
    return !e;
  }

  friend bool operator==(nullptr_t, const one_way_executor& e) noexcept
  {
    return !e;
  }

  friend bool operator!=(const one_way_executor& a, const one_way_executor& b) noexcept
  {
    return !(a == b);
  }

  friend bool operator!=(const one_way_executor& e, nullptr_t) noexcept
  {
    return !!e;
  }

  friend bool operator!=(nullptr_t, const one_way_executor& e) noexcept
  {
    return !!e;
  }

private:
  one_way_executor(impl_base* i) noexcept { context_.impl_ = i; }
  context_type context_;
  const impl_base* get_impl() const { return context_.impl_; }
};

// executor specialized algorithms:

inline void swap(one_way_executor& a, one_way_executor& b) noexcept
{
  a.swap(b);
}

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_ONE_WAY_EXECUTOR_H
