#ifndef STD_EXPERIMENTAL_BITS_EXECUTOR_SHAPE_H
#define STD_EXPERIMENTAL_BITS_EXECUTOR_SHAPE_H

#include <cstddef>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace executor_shape_impl {

template<class>
struct type_check
{
  typedef void type;
};

template<class Executor, class = void>
struct eval
{
  using type = std::size_t;
};

template<class Executor>
struct eval<Executor, typename type_check<typename Executor::shape_type>::type>
{
  using type = typename Executor::shape_type;
};

} // namespace executor_shape_impl

template<class Executor>
struct executor_shape : executor_shape_impl::eval<Executor> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_EXECUTOR_SHAPE_H
