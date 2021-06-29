/**
 * @file seq.h
 */
#pragma once

#include "Map.h"
#include "Value.h"

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

namespace detail {
struct end_seq_exception {};
} // namespace detail

/**
 * @brief end_seq - Can be used to stop a generator function
 *
 * @code
 *  auto generator = [i = 0]() mutable {
 *      if(i == 5)
 *          ltl::end_seq();
 *      return i++;
 *  };
 *
 *  // range = [0, 1, 2, 3, 4]
 *  auto range = ltl::seq(generator);
 * @endcode
 */
inline void end_seq() { throw detail::end_seq_exception{}; }

template <typename F>
/**
 * @brief seq - Transform a generator function into a lazy range
 *
 * @code
 *  auto generator = [i = 0]() mutable {return i++;};
 *  // range1 = [0, 1, 2, 3, 4, ...]
 *  auto range1 = ltl::seq(generator);
 *  // range2 = [0, 1, 2, 3, 4];
 *  auto range2 = ltl::seq(generator) | ltl::take_n(5);
 * @endcode
 *
 * @param f
 */
auto seq(F f) {
    using result = AsPointer<decltype(f())>;
    return valueRange(0) | map_cached([f = std::move(f)](auto &&) mutable -> std::optional<result> {
               try {
                   return result{f()};
               } catch (detail::end_seq_exception) {
                   return std::nullopt;
               }
           });
}

/// @}

} // namespace ltl
