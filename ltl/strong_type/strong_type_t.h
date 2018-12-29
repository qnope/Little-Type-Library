#pragma once
#include "../type_traits.h"
#include <utility>

namespace ltl {
struct ConverterIdentity {
  template <typename T>
  [[nodiscard]] static constexpr T convertToReference(const T &v) {
    return v;
  }

  template <typename T>
  [[nodiscard]] static constexpr T convertFromReference(const T &v) {
    return v;
  }
};
namespace detail {

template <typename T, typename Tag, typename Converter,
          template <typename...> typename... Skills>
class strong_type_t
    : public Skills<strong_type_t<T, Tag, Converter, Skills...>>... {
  static constexpr bool isDefaultConstructible =
      is_default_constructible(type_v<T>);

  template <typename> struct isSameKindTrait : false_t {};
  template <typename C>
  struct isSameKindTrait<strong_type_t<T, Tag, C, Skills...>> : true_t {};

  template <typename U>
  static constexpr bool isSameKind_v = isSameKindTrait<std::decay_t<U>>{};

public:
  template <typename U>
  [[nodiscard]] static isSameKindTrait<std::decay_t<U>> isSameKind(U);

  template <bool dc = isDefaultConstructible, LTL_REQUIRE(dc)>
  explicit constexpr strong_type_t() : m_value{} {}

  template <typename... Args, LTL_REQUIRE(sizeof...(Args) > 0),
            LTL_REQUIRE(!isSameKind_v<Args> && ...)>
  explicit constexpr strong_type_t(Args &&... args)
      : m_value{std::forward<Args>(args)...} {}

  [[nodiscard]] T &get() & { return m_value; }
  [[nodiscard]] constexpr const T &get() const & { return m_value; }
  [[nodiscard]] constexpr T &&get() && { return std::move(m_value); }

  template <typename OtherConverter,
            LTL_REQUIRE(ltl::type_v<OtherConverter> != ltl::type_v<Converter>)>
  [[nodiscard]] constexpr
  operator strong_type_t<T, Tag, OtherConverter, Skills...>() const {
    return strong_type_t<T, Tag, OtherConverter, Skills...>{
        OtherConverter::convertFromReference(
            Converter::convertToReference(m_value))};
  }

private:
  T m_value;
}; // namespace detail
} // namespace detail

template <typename T, typename Tag, template <typename...> typename... Skills>
using strong_type_t =
    detail::strong_type_t<T, Tag, ConverterIdentity, Skills...>;

} // namespace ltl
