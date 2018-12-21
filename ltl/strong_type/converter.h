#pragma once
#include <ratio>

#include "strong_type_t.h"

namespace ltl {
template <typename Ratio> struct ConverterMultiplier {
  template <typename T>
  [[nodiscard]] static constexpr T convertToReference(const T &v) {
    return v * Ratio::num / Ratio::den;
  }

  template <typename T>
  [[nodiscard]] static constexpr T convertFromReference(const T &v) {
    return v * Ratio::den / Ratio::num;
  }
};

namespace detail {
template <typename Base, typename Ratio> struct MultipleOf;

template <typename T, typename Tag, typename Ratio,
          template <typename...> typename... Skills>
struct MultipleOf<strong_type_t<T, Tag, ConverterIdentity, Skills...>, Ratio> {
  using type = strong_type_t<T, Tag, ConverterMultiplier<Ratio>, Skills...>;
};

template <typename T, typename Tag, typename RatioBase, typename RatioToApply,
          template <typename...> typename... Skills>
struct MultipleOf<
    strong_type_t<T, Tag, ConverterMultiplier<RatioBase>, Skills...>,
    RatioToApply> {
  using type = strong_type_t<
      T, Tag,
      ConverterMultiplier<std::ratio<RatioBase::num * RatioToApply::num,
                                     RatioBase::den * RatioToApply::den>>,
      Skills...>;
};

} // namespace detail

template <typename Base, typename Ratio>
using multiple_of = typename detail::MultipleOf<Base, Ratio>::type;

} // namespace ltl
