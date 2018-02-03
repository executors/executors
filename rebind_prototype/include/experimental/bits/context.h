#ifndef STD_EXPERIMENTAL_BITS_CONTEXT_H
#define STD_EXPERIMENTAL_BITS_CONTEXT_H

namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

struct context_t
{
  static constexpr bool is_requirable = false;
  static constexpr bool is_preferable = false;

  //using polymorphic_query_result_type = ?;

  template<class Executor>
    static constexpr bool is_supportable
      = can_query<Executor, context_t>::value;
};

constexpr context_t context;

} // namespace execution
} // inline namespace executors_v1
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_CONTEXT_H
