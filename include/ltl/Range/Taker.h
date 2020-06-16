#pragma once

#include "../ltl.h"
#include "BaseIterator.h"

namespace ltl {

struct TakeNType {
  std::size_t n;
};

struct DropNType {
  std::size_t n;
};

template <typename F> struct TakeWhileType { F f; };

template <typename F> struct DropWhileType { F f; };

inline auto take_n(std::size_t n) { return TakeNType{n}; }
inline auto drop_n(std::size_t n) { return DropNType{n}; }

template <typename F> auto take_while(F f) {
  return TakeWhileType<F>{std::move(f)};
}
template <typename F> auto drop_while(F f) {
  return DropWhileType<F>{std::move(f)};
}

LTL_MAKE_IS_KIND(TakeWhileType, is_take_while_type, IsTakeWhileType, typename);
LTL_MAKE_IS_KIND(DropWhileType, is_drop_while_type, IsDropWhileType, typename);

} // namespace ltl
