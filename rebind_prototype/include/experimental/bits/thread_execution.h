#ifndef STD_EXPERIMENTAL_BITS_THREAD_EXECUTION_H
#define STD_EXPERIMENTAL_BITS_THREAD_EXECUTION_H

namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

struct thread_execution_mapping_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, thread_execution_mapping_t>::value;
};

constexpr thread_execution_mapping_t thread_execution_mapping;

struct new_thread_execution_mapping_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, new_thread_execution_mapping_t>::value;
};

constexpr new_thread_execution_mapping_t new_thread_execution_mapping;

} // namespace execution
} // inline namespace executors_v1
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_THREAD_EXECUTION_H
