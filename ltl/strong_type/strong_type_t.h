#pragma once
#include <utility>

namespace ltl {
template <typename T, typename Tag, typename... Skills> class strong_type_t {
public:
  T &get() & { return m_value; }
  const T &get() const & { return m_value; }

  T &&get() && { return std::move(m_value); }

private:
  T m_value;
};
} // namespace ltl
