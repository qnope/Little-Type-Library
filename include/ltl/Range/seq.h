#pragma once

#include "Map.h"
#include "Value.h"

namespace ltl {

namespace details {
struct end_seq_exception {};
} // namespace details

inline void end_seq() { throw details::end_seq_exception{}; }

template <typename F>
auto seq(F f) {
    using result = AsPointer<decltype(f())>;
    return valueRange(0) | map_cached([f = std::move(f)](auto &&) mutable -> std::optional<result> {
               try {
                   return result{f()};
               } catch (details::end_seq_exception) {
                   return std::nullopt;
               }
           });
}
} // namespace ltl
