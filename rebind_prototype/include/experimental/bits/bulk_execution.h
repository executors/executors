#ifndef STD_EXPERIMENTAL_BITS_BULK_EXECUTION_H
#define STD_EXPERIMENTAL_BITS_BULK_EXECUTION_H

namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

struct bulk_sequenced_execution_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, bulk_sequenced_execution_t>::value;
};

constexpr bulk_sequenced_execution_t bulk_sequenced_execution;

struct bulk_parallel_execution_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, bulk_parallel_execution_t>::value;
};

constexpr bulk_parallel_execution_t bulk_parallel_execution;

struct bulk_unsequenced_execution_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, bulk_unsequenced_execution_t>::value;
};

constexpr bulk_unsequenced_execution_t bulk_unsequenced_execution;

} // namespace execution
} // inline namespace executors_v1
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_BULK_EXECUTION_H
