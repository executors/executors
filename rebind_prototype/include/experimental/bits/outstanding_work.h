#ifndef STD_EXPERIMENTAL_BITS_OUTSTANDING_WORK_H
#define STD_EXPERIMENTAL_BITS_OUTSTANDING_WORK_H

namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

struct outstanding_work_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, outstanding_work_t>::value;
};

constexpr outstanding_work_t outstanding_work;

struct not_outstanding_work_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, not_outstanding_work_t>::value;
};

constexpr not_outstanding_work_t not_outstanding_work;

} // namespace execution
} // inline namespace executors_v1
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_OUTSTANDING_WORK_H
