#pragma once

#include "ltl.h"

namespace ltl {

struct ConverterIdentity {
  template <typename T>[[nodiscard]] static constexpr T convertToReference(const T &v) {
    return v;
  }

  template <typename T>[[nodiscard]] static constexpr T convertFromReference(const T &v) {
    return v;
  }
};
namespace detail {

template <typename T, typename Tag, typename Converter,
          template <typename...> typename... Skills>
class strong_type_t : public Skills<strong_type_t<T, Tag, Converter, Skills...>>... {
  static constexpr bool isDefaultConstructible = is_default_constructible(type_v<T>);

  template <typename> struct isSameKindTrait : false_t {};
  template <typename C>
  struct isSameKindTrait<strong_type_t<T, Tag, C, Skills...>> : true_t {};

  template <typename U> static constexpr isSameKindTrait<std::decay_t<U>> isSameKind_v{};

public:
  template <typename U>
  [[nodiscard]] static isSameKindTrait<std::decay_t<U>> isSameKind(U);

  template <bool dc = isDefaultConstructible, typename = std::enable_if_t<dc>>
  explicit constexpr strong_type_t() : m_value{} {}

  template <typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 0)>,
            typename = std::enable_if_t<((!isSameKind_v<Args>)&&...)>>
  explicit constexpr strong_type_t(Args &&... args) : m_value{FWD(args)...} {}

  [[nodiscard]] constexpr T &get() & { return m_value; }
  [[nodiscard]] constexpr const T &get() const & { return m_value; }
  [[nodiscard]] constexpr T &&get() && { return std::move(m_value); }

  template <typename OtherConverter,
            typename = std::enable_if_t<type_v<OtherConverter> != type_v<Converter>>>
  [[nodiscard]] constexpr
  operator strong_type_t<T, Tag, OtherConverter, Skills...>() const {
    return strong_type_t<T, Tag, OtherConverter, Skills...>{
        OtherConverter::convertFromReference(Converter::convertToReference(m_value))};
  }

private:
  T m_value;
}; // namespace detail
} // namespace detail

template <typename T, typename Tag, template <typename...> typename... Skills>
using strong_type_t = detail::strong_type_t<T, Tag, ConverterIdentity, Skills...>;

////////////////// Arithmetic strong types
#define OP(name, op)                                                                     \
  template <typename T> struct name : crtp<T, name> {                                    \
    template <typename T2> T &operator LPL_CAT(op, =)(const T2 &other) {                 \
      typed_static_assert(this->underlying().isSameKind(other));                         \
      this->underlying().get() LPL_CAT(op, =) static_cast<T>(other).get();               \
      return this->underlying();                                                         \
    }                                                                                    \
                                                                                         \
    template <typename T2>                                                               \
    [[nodiscard]] constexpr friend T operator op(const T &a, const T2 &b) {              \
      typed_static_assert(a.isSameKind(b));                                              \
      return T{a.get() op static_cast<T>(b).get()};                                      \
    }                                                                                    \
  };

OP(Addable, +)
OP(Subtractable, -)
OP(Multipliable, *)
OP(Dividable, /)
OP(Moduloable, %)

#undef OP

#define OP(name, op)                                                                     \
  template <typename T> struct name : crtp<T, name> {                                    \
    constexpr T &operator op() {                                                         \
      op this->underlying().get();                                                       \
      return this->underlying();                                                         \
    }                                                                                    \
                                                                                         \
    constexpr T operator op(int) {                                                       \
      this->underlying().get() op;                                                       \
      return *this->underlying();                                                        \
    }                                                                                    \
  };

OP(Incrementable, ++)
OP(Decrementable, --)

#undef OP

/////////////// Equality for strong types
#define OP(name, op)                                                                     \
  template <typename T> struct name {                                                    \
    [[nodiscard]] constexpr friend bool operator op(const T &a, const T &b) {            \
      return a.get() op b.get();                                                         \
    }                                                                                    \
                                                                                         \
    template <typename T2>                                                               \
    [[nodiscard]] constexpr friend bool operator op(const T &a, const T2 &b) {           \
      typed_static_assert(a.isSameKind(b));                                              \
      return a.get() op static_cast<T>(b).get();                                         \
    }                                                                                    \
  };

OP(GreaterThan, >)
OP(LessThan, <)
OP(GreaterThanEqual, >=)
OP(LessThanEqual, <=)

namespace detail {
OP(LTLSTEquality, ==)
OP(LTLSTInequality, !=)
} // namespace detail

template <typename T>
struct EqualityComparable : detail::LTLSTEquality<T>, detail::LTLSTInequality<T> {};

template <typename T>
struct AllComparable : EqualityComparable<T>,
                       GreaterThan<T>,
                       LessThan<T>,
                       GreaterThanEqual<T>,
                       LessThanEqual<T> {};

#undef OP

//////////////////// Stream for strong types
template <typename T> struct OStreamable {
  friend std::ostream &operator<<(std::ostream &s, const T &v) { return s << v.get(); }
};

/////////////////// StrongType Converter
template <typename Ratio> struct ConverterMultiplier {
  template <typename T>[[nodiscard]] static constexpr T convertToReference(const T &v) {
    return v * Ratio::num / Ratio::den;
  }

  template <typename T>[[nodiscard]] static constexpr T convertFromReference(const T &v) {
    return v * Ratio::den / Ratio::num;
  }
};

namespace detail {
template <typename Base, typename Converter> struct AddConverter;

template <typename T, typename Tag, typename Converter,
          template <typename...> typename... Skills>
struct AddConverter<strong_type_t<T, Tag, ConverterIdentity, Skills...>, Converter> {
  using type = strong_type_t<T, Tag, Converter, Skills...>;
};

template <typename Base, typename Ratio> struct MultipleOf;

template <typename T, typename Tag, typename Ratio,
          template <typename...> typename... Skills>
struct MultipleOf<strong_type_t<T, Tag, ConverterIdentity, Skills...>, Ratio> {
  using type = strong_type_t<T, Tag, ConverterMultiplier<Ratio>, Skills...>;
};

template <typename T, typename Tag, typename RatioBase, typename RatioToApply,
          template <typename...> typename... Skills>
struct MultipleOf<strong_type_t<T, Tag, ConverterMultiplier<RatioBase>, Skills...>,
                  RatioToApply> {
  using type =
      strong_type_t<T, Tag,
                    ConverterMultiplier<std::ratio<RatioBase::num * RatioToApply::num,
                                                   RatioBase::den * RatioToApply::den>>,
                    Skills...>;
};
} // namespace detail

template <typename Base, typename Converter>
using add_converter = typename detail::AddConverter<Base, Converter>::type;

template <typename Base, typename Ratio>
using multiple_of = typename detail::MultipleOf<Base, Ratio>::type;

} // namespace ltl
