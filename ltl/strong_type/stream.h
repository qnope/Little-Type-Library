#pragma once
#include "strong_type_t.h"
#include <iostream>

namespace ltl {
template <typename T> struct OStreamable {
  friend std::ostream &operator<<(std::ostream &s, const T &v) {
    return s << v.get();
  }
};

} // namespace ltl
