#ifndef STD_EXPERIMENTAL_BITS_CONTINUATION_H
#define STD_EXPERIMENTAL_BITS_CONTINUATION_H

namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

struct continuation_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, continuation_t>::value;
};

constexpr continuation_t continuation;

struct not_continuation_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  using polymorphic_query_result_type = bool;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, not_continuation_t>::value;
};

constexpr not_continuation_t not_continuation;

} // namespace execution
} // inline namespace executors_v1
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_CONTINUATION_H
