#ifndef STD_EXPERIMENTAL_BITS_EXECUTOR_BULK_FORWARD_PROGRESS_GUARANTEE_H
#define STD_EXPERIMENTAL_BITS_EXECUTOR_BULK_FORWARD_PROGRESS_GUARANTEE_H

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace executor_bulk_forward_progress_guarantee_impl {

template<class>
struct type_check
{
  typedef void type;
};

template<class Executor, class = void>
struct eval
{
  using type = bulk_unsequenced_execution;
};

template<class Executor>
struct eval<Executor, typename type_check<typename Executor::bulk_forward_progress_guarantee>::type>
{
  using type = typename Executor::bulk_forward_progress_guarantee;
};

} // namespace executor_bulk_forward_progress_guarantee_impl

template<class Executor>
struct executor_bulk_forward_progress_guarantee : executor_bulk_forward_progress_guarantee_impl::eval<Executor> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_EXECUTOR_BULK_FORWARD_PROGRESS_GUARANTEE_H
