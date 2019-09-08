#pragma once

#include "ltl.h"
#include <variant>

namespace ltl {
template <typename V, typename... Fs> decltype(auto) match(V &&v, Fs &&... fs) {
  return ::std::visit(overloader{FWD(fs)...}, FWD(v));
}
} // namespace ltl
