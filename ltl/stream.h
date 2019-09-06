#pragma once
#include "ltl.h"
#include <iostream>
#include <iterator>

namespace ltl {
struct Ostream_Wrapper {
  Ostream_Wrapper(std::ostream &stream) : stream{stream} {}
  std::ostream &stream;
};

template <typename T> Ostream_Wrapper &operator<<(Ostream_Wrapper &s, T &&t) {
  if_constexpr(decay_from(t) == ltl::type_v<char *> ||
               decay_from(t) == ltl::type_v<const char *> ||
               decay_from(t) == ltl::type_v<std::string>) {
    s.stream << FWD(t);
  }
  else_if_constexpr(is_iterable(FWD(t))) {
    auto it = std::begin(FWD(t));
    auto itNext = it;
    auto end = std::end(FWD(t));
    ++itNext;
    while (it != end) {
      s.stream << *it;
      if (itNext != end) {
        s.stream << " ";
        ++itNext;
      }
      ++it;
    }
  }

  else_if_constexpr(is_optional(FWD(t))) {
    if (FWD(t)) {
      s.stream << *t;
    }
  }

  else s.stream << FWD(t);

  return s;
}

Ostream_Wrapper cout(std::cout);
} // namespace ltl
