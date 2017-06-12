#ifndef STD_EXPERIMENTAL_BITS_EXECUTOR_INDEX_H
#define STD_EXPERIMENTAL_BITS_EXECUTOR_INDEX_H

#include <experimental/bits/executor_shape.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace executor_index_impl {

template<class>
struct type_check
{
  typedef void type;
};

template<class Executor, class = void>
struct eval
{
  using type = executor_shape_t<Executor>;
};

template<class Executor>
struct eval<Executor, typename type_check<typename Executor::index_type>::type>
{
  using type = typename Executor::index_type;
};

} // namespace executor_index_impl

template<class Executor>
struct executor_index : executor_index_impl::eval<Executor> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_EXECUTOR_INDEX_H
