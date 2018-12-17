#pragma once
#include "../type_trait.h"
#include <utility>

namespace ltl {
template <typename T, typename Tag, template <typename...> typename... Skills>
class strong_type_t : Skills<strong_type_t<T, Tag, Skills...>>... {
public:
  static constexpr bool isDefaultConstructible =
      is_default_constructible(type_v<T>);

  template <bool dc = isDefaultConstructible, typename = std::enable_if_t<dc>>
  explicit constexpr strong_type_t() : m_value{} {}

  template <typename... Args,
            typename = std::enable_if_t<(sizeof...(Args) > 0)>,
            typename = std::enable_if_t<
                ((ltl::type_v<std::decay_t<Args>> !=
                  ltl::type_v<strong_type_t<T, Tag, Skills...>>)&&...)>>
  explicit constexpr strong_type_t(Args &&... args)
      : m_value{std::forward<Args>(args)...} {}

  [[nodiscard]] T &get() & { return m_value; }
  [[nodiscard]] constexpr const T &get() const & { return m_value; }

  [[nodiscard]] constexpr T &&get() && { return std::move(m_value); }

private:
  T m_value;
};
} // namespace ltl
