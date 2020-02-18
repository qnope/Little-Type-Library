#pragma once

#include "../ltl.h"
#include "BaseIterator.h"

namespace ltl {

struct TakerType {
  std::size_t n;
};
inline auto take_n(std::size_t n) { return TakerType{n}; }

} // namespace ltl
