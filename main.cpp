#include <array>
#include <cassert>
#include <functional>
#include <string>
#include <unordered_map>

#include "ltl/StrongType.h"
#include "ltl/VariantUtils.h"
#include "ltl/algos.h"
#include "ltl/condition.h"
#include "ltl/functional.h"
#include "ltl/operator.h"
#include "ltl/optional_type.h"
#include "ltl/traits.h"

#include "ltl/Range/DefaultView.h"
#include "ltl/Range/Value.h"
#include "ltl/Range/enumerate.h"

void bool_test() {
  static_assert(false_v == false_v);
  static_assert(false_v != true_v);
  static_assert(true_v != false_v);
  static_assert(true_v == true_v);

  static_assert(true_v && true_v);
  static_assert(!(false_v && false_v));
  static_assert(!(false_v && true_v));
  static_assert(!(true_v && false_v));

  static_assert(!(false_v || false_v));
  static_assert(false_v || true_v);
  static_assert(true_v || false_v);
  static_assert(true_v || true_v);

  static_assert(!false_v);
  static_assert(!!true_v);
  static_assert(true_v);

  static_assert(ltl::not_([] { return false_v; })());
}

void type_test() {
  static_assert(ltl::type_v<int> == ltl::type_v<int>);
  static_assert(ltl::type_v<int> != ltl::type_v<double>);
  static_assert(!(ltl::type_v<int> == ltl::type_v<double>));
  static_assert(type_from(true_v == true_v) == ltl::type_v<ltl::true_t>);
}

void number_test() {
  constexpr ltl::number_t<1> one;
  constexpr auto two = 2_n;
  static_assert(3_n == one + two && one < two);
  static_assert(1_n + 4_n == 5_n);
  static_assert(4_n + 1_n != 6_n);
  static_assert(-1_n + 5_n == 4_n);
  static_assert(5_n + -3_n == 2_n);

  static_assert(2_n * 3_n == 6_n);
  static_assert(!(3_n * 0_n != 0_n));
  static_assert(8_n * -5_n == -40_n);

  static_assert(1024_n - 25_n == 999_n);
  static_assert(1024_n / 2_n == 512_n);

  static_assert((8_n & 2_n) == 0_n);
  static_assert(((6_n & 1_n) == 0_n) && ((6_n & 2_n) == 2_n));
  static_assert((8_n | 7_n) == 15_n);
  static_assert((8_n ^ 15_n) == 7_n);

  static_assert(ltl::max(5_n, 3_n, 8_n, 4_n) == 8_n);
  static_assert(ltl::min(4_n, -8_n, 8_n, 4_n) == -8_n);
}

void constexpr_tuple_test() {
  constexpr ltl::tuple_t tuple{5, 3.0};

  static_assert(decay_from(tuple) == ltl::type_v<ltl::tuple_t<int, double>>);
  static_assert(apply(tuple, [](auto a, auto b) { return a + b; }) == 8.0);

  static_assert(tuple.get(0_n) == 5 && tuple.get(1_n) == 3.0);
  static_assert(ltl::tuple_t{5, 3.0}.get(0_n) == 5 &&
                ltl::tuple_t{5, 3.0}.get(1_n) == 3.0);

  static_assert(tuple[0_n] == 5 && tuple[1_n] == 3.0);
  static_assert(ltl::tuple_t{5, 3.0}[0_n] == 5 &&
                ltl::tuple_t{5, 3.0}[1_n] == 3.0);

  static_assert(tuple == ltl::tuple_t<int, double>{5, 3.0});
  static_assert(tuple != ltl::tuple_t<int, double>{5, 3.1});

  static_assert(tuple < ltl::tuple_t{5, 6.0});
  static_assert(tuple < ltl::tuple_t{6, 2.0});
  static_assert(tuple <= ltl::tuple_t{5, 3.0});

  static_assert(tuple > ltl::tuple_t{5, 2.0});
  static_assert(tuple >= ltl::tuple_t{5, 2.8});

  static_assert(
      ltl::type_list_v<int, double, char> ==
      ltl::type_list_v<double>.push_back(ltl::type_v<char>).push_front(ltl::type_v<int>));

  static_assert(
        ltl::type_list_v<float, int, double, double, char,
                         double>.pop_front().pop_back().pop_back().pop_back().push_back(ltl::type_v<char>)
        == ltl::type_list_v<int, double, char>);

  constexpr auto number_list = ltl::number_list_v<2, 3, 4>;

  static_assert(number_list.push_back(5_n).push_back(6_n) ==
                ltl::number_list_v<2, 3, 4, 5, 6>);
  static_assert(number_list.push_front(1_n).push_front(0_n) ==
                ltl::number_list_v<0, 1, 2, 3, 4>);

  static_assert(number_list.pop_back().pop_back() == ltl::number_list_v<2>);
  static_assert(number_list.pop_front().pop_front() == ltl::number_list_v<4>);
  static_assert(ltl::build_index_sequence(5_n) ==
                ltl::number_list_v<0, 1, 2, 3, 4>);
}

void tuple_test() {
  ltl::tuple_t _tuple{5, 3.0};

  assert(apply(_tuple, [](auto a, auto b) { return a + b; }) == 8.0);
  for_each(_tuple, ltl::overloader{[](int v) { assert(v == 5); },
                                   [](double v) { assert(v == 3.0); }});
  auto tuple = _tuple.push_front(8).push_back(95);
  assert(tuple[3_n] == 95);
  tuple[1_n] = 25;
  tuple[2_n] = 25.3;

  for_each(tuple.extract(1_n, 2_n),
           ltl::overloader{[](int v) { assert(v == 25); },
                           [](double v) { assert(v == 25.3); }});
  auto [a, b, c, d] = tuple;
  auto &[a2, b2, c2, d2] = tuple;
  const auto [a3, b3, c3, d3] = tuple;
  const auto [a4, b4, c4, d4] = std::as_const(tuple);
  const auto &[a5, b5, c5, d5] = tuple;
  const auto &[a6, b6, c6, d6] = std::as_const(tuple);
  auto &&[a8, b8, c8, d8] = std::move(tuple);
  auto [a7, b7, c7, d7] = std::move(tuple);
  assert(a == tuple[0_n]);
  assert(b == tuple[1_n]);
  assert(c == tuple[2_n]);
  assert(d == tuple[3_n]);

  assert(&a2 == &tuple[0_n]);
  assert(&b2 == &tuple[1_n]);
  assert(&c2 == &tuple[2_n]);
  assert(&d2 == &tuple[3_n]);

  assert(a3 == a);
  assert(b3 == b);
  assert(c3 == c);
  assert(d3 == d);

  assert(a4 == a);
  assert(b4 == b);
  assert(c4 == c);
  assert(d4 == d);

  assert(&a5 == &tuple[0_n]);
  assert(&b5 == &tuple[1_n]);
  assert(&c5 == &tuple[2_n]);
  assert(&d5 == &tuple[3_n]);

  assert(&a6 == &tuple[0_n]);
  assert(&b6 == &tuple[1_n]);
  assert(&c6 == &tuple[2_n]);
  assert(&d6 == &tuple[3_n]);

  assert(a7 == a);
  assert(b7 == b);
  assert(c7 == c);
  assert(d7 == d);

  assert(&a8 == &tuple[0_n]);
  assert(&b8 == &tuple[1_n]);
  assert(&c8 == &tuple[2_n]);
  assert(&d8 == &tuple[3_n]);

  int ta, tb, td;
  double tc;
  ltl::tie(ta, tb, tc, td) = tuple;
  assert(ta == tuple[0_n]);
  assert(tb == tuple[1_n]);
  assert(tc == tuple[2_n]);
  assert(td == tuple[3_n]);

  int lvalue = 25;
  ltl::tuple_t firstTuple{28, lvalue, std::ref(lvalue)};
  ltl::tuple_t secondTuple{34, std::cref(lvalue)};

  typed_static_assert(
      (ltl::type_v<ltl::tuple_t<int, int, int &>> == type_from(firstTuple)));
  typed_static_assert(
      (ltl::type_v<ltl::tuple_t<int, const int &>> == type_from(secondTuple)));

  auto firstAdd = firstTuple + secondTuple;
  auto secondAdd = firstTuple + std::move(secondTuple);
  auto thirdAdd = std::move(firstTuple) + std::move(secondTuple);
  auto lastAdd = std::move(secondTuple) + std::move(firstTuple);

  typed_static_assert(
      (ltl::type_v<ltl::tuple_t<int, int, int &, int, const int &>> ==
       type_from(firstAdd)));
  typed_static_assert(
      (ltl::type_v<ltl::tuple_t<int, int, int &, int, const int &>> ==
       type_from(secondAdd)));
  typed_static_assert(
      (ltl::type_v<ltl::tuple_t<int, int, int &, int, const int &>> ==
       type_from(thirdAdd)));
  typed_static_assert(
      (ltl::type_v<ltl::tuple_t<int, const int &, int, int, int &>> ==
       type_from(lastAdd)));
}

void tuple_test_algo() {
  {
    int a;
    ltl::tuple_t<int &, double, ltl::type_t<int>, int> tuple(
        a, 5.0, ltl::type_v<int>, 5);
    typed_static_assert(ltl::contains_type(tuple, ltl::type_v<int>));
    typed_static_assert(ltl::contains_type(tuple, ltl::type_v<int &>));
    typed_static_assert(!ltl::contains_type(tuple, ltl::type_v<double &>));
    typed_static_assert(
        ltl::contains_type(tuple, ltl::type_v<ltl::type_t<int>>));
    typed_static_assert(!ltl::contains_type(tuple, ltl::type_v<char>));

    typed_static_assert(ltl::count_type(tuple, ltl::type_v<int>) == 1_n);
    typed_static_assert(ltl::count_type(tuple, ltl::type_v<char>) == 0_n);

    typed_static_assert(*ltl::find_type(tuple, ltl::type_v<double>) == 1_n);
    typed_static_assert(*ltl::find_type(tuple, ltl::type_v<int>) == 3_n);
  }

  {
    ltl::type_list_t<int, double, int, int> tuple;
    typed_static_assert(ltl::contains_type(tuple, ltl::type_v<int>));
    typed_static_assert(!ltl::contains_type(tuple, ltl::type_v<char>));
    typed_static_assert(ltl::count_type(tuple, ltl::type_v<int>) == 3_n);
    typed_static_assert(*ltl::find_type(tuple, ltl::type_v<int>) == 0_n);
    typed_static_assert(*ltl::find_type(tuple, ltl::type_v<int>, 0_n + 1_n) ==
                        2_n);
    typed_static_assert(*ltl::find_type(tuple, ltl::type_v<double>) == 1_n);
  }

  {
    ltl::type_list_t<int, double *, int *, int, char *> tuple;
    typed_static_assert(ltl::contains_if_type(tuple, ltl::is_pointer));
    typed_static_assert(ltl::count_if_type(tuple, ltl::is_pointer) == 3_n);
    typed_static_assert(*ltl::find_if_type(tuple, ltl::is_pointer) == 1_n);
    typed_static_assert(*ltl::find_if_type(tuple, ltl::is_pointer, 2_n) == 2_n);
  }

  {
    ltl::type_list_t<int, int, unsigned int, char> tuple1;
    typed_static_assert(ltl::all_of_type(tuple1, ltl::is_integral));
    typed_static_assert(ltl::none_of_type(tuple1, ltl::is_floating_point));
    typed_static_assert(ltl::any_of_type(tuple1, ltl::is_unsigned));
  }

  {
    ltl::type_list_t<int, int, double, unsigned char> tuple;
    ltl::type_list_t<int *, int *, double *, unsigned char *> ptrs;
    typed_static_assert(type_from(ptrs) == type_from(ltl::transform_type(
                                               tuple, ltl::add_pointer)));
  }

  {
    ltl::tuple_t<int, int *, double, int, double *, char, char, char *> tuple;
    typed_static_assert(
        (type_from(ltl::unique_type(tuple)) ==
         ltl::type_v<
             ltl::type_list_t<int, int *, double, double *, char, char *>>));
  }

  {
    constexpr ltl::tuple_t<int, double, char> tuple1{8, 5.0, 3};
    static_assert(16 == ltl::accumulate_type(tuple1));

    constexpr ltl::type_list_t<int, double, char> tuple2;
    constexpr auto accumulator = [](auto... types) {
      return (... + ltl::tuple_t{ltl::add_pointer(types)});
    };
    static_assert(type_from(ltl::accumulate_type(tuple2, accumulator)) ==
                  type_from(ltl::transform_type(tuple2, ltl::add_pointer)));
  }
}

void push_pop_test() {
  using namespace std::literals;
  ltl::tuple_t all_pop{"0"s, "1"s, "2"s};
  auto l02_pop = all_pop.extract(0_n, 2_n);

  auto l12_pop = all_pop.pop_front();
  auto l01_pop = std::move(all_pop).pop_back();

  for_each(l02_pop, [](auto s) { assert(s == "0" || s == "2"); });
  for_each(l12_pop, [](auto s) { assert(s == "1" || s == "2"); });
  for_each(l01_pop, [](auto s) { assert(s == "0" || s == "1"); });
  for_each(all_pop, [](auto s) { assert(s == "" || s == "2"); });

  ltl::tuple_t all_push{"1"s, "2"s};
  auto l123 = all_push.push_back("3"s);
  auto l012 = std::move(all_push).push_front("0"s);

  for_each(all_push, [](auto s) { assert(s == ""); });
  for_each(l123, [](auto s) { assert(s == "1" || s == "2" || s == "3"); });
  for_each(l012, [](auto s) { assert(s == "0" || s == "1" || s == "2"); });
}

void tuple_reference_test() {
  int a{0};
  auto base = ltl::tuple_t{a, std::ref(a), 0};
  auto pushedBackRef = base.push_back(std::ref(a));
  auto pushedBack = base.push_back(a);
  auto pushedFrontRef = base.push_front(std::ref(a));
  auto pushedFront = base.push_front(a);
  auto popBack = base.pop_back();
  auto popFront = base.pop_front();

  static_assert(type_from(base) == ltl::type_v<ltl::tuple_t<int, int &, int>>);

  static_assert(type_from(pushedBackRef) ==
                ltl::type_v<ltl::tuple_t<int, int &, int, int &>>);
  static_assert(type_from(pushedBack) ==
                ltl::type_v<ltl::tuple_t<int, int &, int, int>>);
  static_assert(type_from(pushedFrontRef) ==
                ltl::type_v<ltl::tuple_t<int &, int, int &, int>>);
  static_assert(type_from(pushedFront) ==
                ltl::type_v<ltl::tuple_t<int, int, int &, int>>);

  static_assert(type_from(popBack) == ltl::type_v<ltl::tuple_t<int, int &>>);
  static_assert(type_from(popFront) == ltl::type_v<ltl::tuple_t<int &, int>>);

  ltl::tuple_t testBracket{a, std::ref(a), 0};

  static_assert(type_from(testBracket) ==
                ltl::type_v<ltl::tuple_t<int, int &, int>>);
  static_assert(type_from(testBracket[1_n]) == ltl::type_v<int &>);
  static_assert(type_from(testBracket[0_n]) == ltl::type_v<int &>);
  static_assert(type_from(testBracket[2_n]) == ltl::type_v<int &>);

  static_assert(type_from(std::move(testBracket)[1_n]) == ltl::type_v<int &>);
  static_assert(type_from(std::move(testBracket)[0_n]) == ltl::type_v<int &&>);
  static_assert(type_from(std::move(testBracket)[2_n]) == ltl::type_v<int &&>);
}

void test_is_valid() {
  using namespace std::literals;
  constexpr auto additionnable = IS_VALID((x, y), x + y);

  int integer;
  typed_static_assert(additionnable(integer, 8));
  typed_static_assert(!additionnable(integer, "lol"s));
}

void test_trait() {
  struct Default {
    Default() = default;
  };

  struct NonDefault {
    NonDefault() = delete;
  };

  static_assert(ltl::is_default_constructible(ltl::type_v<Default>));
  static_assert(!ltl::is_default_constructible(ltl::type_v<NonDefault>));
  static_assert(ltl::is_void(ltl::type_v<void>));
  static_assert(!ltl::is_void(ltl::type_v<int>));
  static_assert(ltl::is_null_pointer(type_from(nullptr)));
  static_assert(ltl::is_integral(ltl::type_v<int>));
  static_assert(ltl::is_floating_point(ltl::type_v<double>));
  static_assert(ltl::is_array(ltl::type_v<float[]>));
  static_assert(ltl::is_floating_point(5.0));
  static_assert(!ltl::is_floating_point(5));
  static_assert(ltl::is_integral(5));
  static_assert(!ltl::is_integral(5.0));
  static_assert(ltl::is_class(ltl::type_v<Default>));
  static_assert(ltl::is_lvalue_reference(ltl::type_v<int &>));
  static_assert(ltl::is_rvalue_reference(ltl::type_v<int &&>));
  static_assert(ltl::is_const(ltl::type_v<const int>));
  static_assert(ltl::is_trivially_destructible(ltl::type_v<Default>));
  static_assert(ltl::extent(ltl::type_v<float[5][3]>, 1_n) == 3_n);
  static_assert(ltl::extent(ltl::type_v<float[5][3]>, 0_n) == 5_n);
  static_assert(ltl::extent(ltl::type_v<float[5][3]>) == 5_n);
  static_assert(ltl::add_pointer(ltl::type_v<int>) == ltl::type_v<int *>);
  static_assert(ltl::is_iterable(ltl::type_v<std::vector<int>>));
  static_assert(!ltl::is_iterable(ltl::type_v<int>));
  static_assert(ltl::is_iterable(ltl::type_v<std::array<int, 1>>));
  typed_static_assert(!ltl::is_generic_callable(std::any{}));

  auto lambda_int = [](int) {};
  auto lambda_double = [](double) {};
  auto lambda_auto = [](auto) {};

  typed_static_assert(
      !ltl::is_generic_callable(ltl::overloader{lambda_int, lambda_double}));
  typed_static_assert(!ltl::is_generic_callable(lambda_int));
  typed_static_assert(ltl::is_generic_callable(lambda_auto));
  typed_static_assert(ltl::is_generic_callable(
      ltl::overloader{lambda_auto, lambda_int, lambda_double}));

  typed_static_assert(ltl::copy_qualifier<double>(ltl::type_v<const int &>) ==
                      ltl::type_v<const double &>);
  typed_static_assert(ltl::copy_qualifier<double>(ltl::type_v<int &>) ==
                      ltl::type_v<double &>);
  typed_static_assert(ltl::copy_qualifier<double>(ltl::type_v<int &&>) ==
                      ltl::type_v<double &&>);

  {
    int lvalue;
    const int clvalue = 0;
    std::array<int, 5> array{};
    std::vector<double> array2{};
    std::optional<int> opt;
    static_assert(!ltl::is_rvalue_reference(lvalue));
    static_assert(ltl::is_rvalue_reference(std::move(lvalue)));
    static_assert(ltl::is_rvalue_reference(5));
    static_assert(ltl::is_const(clvalue));
    static_assert(ltl::is_optional(opt));
    static_assert(!ltl::is_iterable(opt));
    static_assert(ltl::is_iterable(array));
    static_assert(ltl::is_iterable(array2));
  }

  {
    static_assert(ltl::is_type(ltl::type_v<int>)(8));
    static_assert(ltl::is_type(ltl::type_v<int>)(ltl::type_v<int>));

    static_assert(!ltl::is_type(ltl::type_v<int>)(ltl::type_v<long>));

    struct Base {};
    struct Derived : Base {};
    struct NotDerived {};
    Derived d;
    NotDerived nd;

    static_assert(ltl::is_derived_from(ltl::type_v<Base>)(d));
    static_assert(
        ltl::is_derived_from(ltl::type_v<Base>)(ltl::type_v<Derived>));

    static_assert(!ltl::is_derived_from(ltl::type_v<Base>)(nd));
    static_assert(
        !ltl::is_derived_from(ltl::type_v<Base>)(ltl::type_v<NotDerived>));
  }
}

void test_qualifier() {
  int a = 0;
  int &refA = a;
  constexpr auto qualifier = qualifier_from(refA);
  const double b = 8.0;
  constexpr auto normalType = type_from(a);
  constexpr auto constRefType = qualifier_from(b) + normalType + qualifier +
                                ltl::qualifier_v<ltl::qualifier_enum::VOLATILE>;

  typed_static_assert(normalType == ltl::type_v<int>);
  typed_static_assert(constRefType == ltl::type_v<volatile const int &>);
  static_assert(ltl::getQualifierEnum(constRefType) ==
                (ltl::qualifier_enum::CONST | ltl::qualifier_enum::LVALUE_REF |
                 ltl::qualifier_enum::VOLATILE));
}

using Float =
    ltl::strong_type_t<float, struct FloatTag, ltl::EqualityComparable,
                       ltl::GreaterThan, ltl::LessThan, ltl::Addable,
                       ltl::Subtractable>;

using Meter =
    ltl::strong_type_t<float, struct DistanceTag, ltl::EqualityComparable,
                       ltl::GreaterThan, ltl::LessThan, ltl::Addable,
                       ltl::Subtractable, ltl::OStreamable>;

using Km = ltl::multiple_of<Meter, std::ratio<1000>>;
using Dm = ltl::multiple_of<Km, std::ratio<1, 100>>;

constexpr float pi = 3.1415926535f;

struct ConverterRadianDegree {
  [[nodiscard]] static constexpr float convertToReference(float degree) {
    return degree * pi / 180.0f;
  }

  [[nodiscard]] static constexpr float convertFromReference(float radians) {
    return radians * 180.0f / pi;
  }
};

using radians =
    ltl::strong_type_t<float, struct AngleTag, ltl::EqualityComparable>;
using degrees = ltl::add_converter<radians, ConverterRadianDegree>;

void test_strong_type() {
  constexpr Float floatDefault{};
  constexpr Float floatSix{6.0f};
  constexpr Float floatCopy(floatSix);
  Float floatEquallyCopied;
  floatEquallyCopied = floatCopy;
  static_assert(floatSix == floatCopy);
  static_assert(floatSix != floatDefault);
  static_assert(floatSix > floatDefault);
  static_assert(floatDefault < floatSix);
  static_assert(floatSix + Float{6.0f} == Float{12.0f});
  static_assert(floatSix - Float{6.0f} == Float{0.0f});

  constexpr Km oneKilometer{1.0f};
  constexpr Meter oneKilometerInMeter{oneKilometer};
  Dm oneKilometerInDecimeter{oneKilometerInMeter};
  static_assert(oneKilometer == Meter{1000.0f});
  static_assert(oneKilometer != Meter{1200.0f});
  static_assert(oneKilometerInMeter < Meter{1200.0f});
  static_assert(oneKilometer < Meter{1200.0f});
  static_assert(oneKilometer + oneKilometerInMeter == Meter{2000.0f});
  static_assert(Meter{1200.0f} == Km{1.2f});
  static_assert(type_from(oneKilometer + oneKilometerInMeter) ==
                ltl::type_v<Km>);

  // ratio<1000, 1000> is not the same type as ratio<1>, but it is equivalent
  static_assert(ltl::type_v<ltl::multiple_of<Km, std::ratio<1, 1000>>> ==
                ltl::type_v<ltl::multiple_of<Meter, std::ratio<1000, 1000>>>);

  static_assert(oneKilometer.get() * 1000 == oneKilometerInMeter.get());
  assert(oneKilometer.get() * 100 == oneKilometerInDecimeter.get());
  oneKilometerInDecimeter += Meter{30.0f} + Km{1.0f};
  assert(oneKilometerInDecimeter == Meter{2030.0f} &&
         oneKilometerInDecimeter == Km{2.03f} &&
         oneKilometerInDecimeter == Dm{203.0f});

  constexpr radians rad{pi};
  constexpr degrees deg(rad);
  static_assert(rad == deg);
  static_assert(rad.get() == deg.get() * pi / 180.0f);
  static_assert(deg.get() == rad.get() * 180.0f / pi);
}

void test_algos() {
  using namespace std::literals;
  std::array<int, 5> odds = {3, 5, 7, 9, 11};
  std::unordered_map<std::string, int> unordered_map = {{"one"s, 1},
                                                        {"two"s, 2}};
  auto areEvens = [](auto x) { return (x & 1) == 0; };
  auto areOdds = [](auto x) { return (x & 1) != 0; };
  auto isSuperiorTo = [](auto n) { return [n](auto x) { return x > n; }; };

  assert(ltl::all_of(odds, areOdds));
  assert(ltl::any_of(odds, isSuperiorTo(10)));
  assert(ltl::none_of(odds, areEvens));
  assert(ltl::count(odds, 5) == 1);
  assert(ltl::count(odds, 1) == 0);
  assert(ltl::count_if(odds, isSuperiorTo(4)) == 4);
  assert(*ltl::find_if(odds, isSuperiorTo(10)) == odds.begin() + 4);
  assert(ltl::accumulate(odds, 0) == 3 + 5 + 7 + 9 + 11);
  assert(ltl::accumulate(std::move(odds), 0) == 3 + 5 + 7 + 9 + 11);
  assert(ltl::computeMean(std::move(odds)) == (3 + 5 + 7 + 9 + 11) / 5);
  assert(ltl::computeMean(std::vector<int>{}) == std::nullopt);
  assert(ltl::contains(odds, 9));
  assert(!ltl::contains(odds, 10));
  assert(ltl::contains_map(unordered_map, "one"));
  assert(ltl::contains_map(unordered_map, "three") == false);
  assert(!ltl::find_map_value(unordered_map, "three"));
  assert(*ltl::find_map_value(unordered_map, "one") == 1);
  assert(ltl::find_map_ptr(unordered_map, "one") == &unordered_map["one"]);
  assert(ltl::find_map_ptr(unordered_map, "three") == nullptr);

  std::array<int, 5> reverseOdds = {11, 9, 7, 5, 3};
  assert(!ltl::equal(reverseOdds, odds));
  ltl::reverse(odds);
  assert(ltl::equal(reverseOdds, odds));

  auto &oddsRef = ltl::sort(odds);
  assert(&oddsRef == &odds);
  assert(ltl::equal(oddsRef, std::array<int, 5>{3, 5, 7, 9, 11}));

  auto v = ltl::sort(std::vector{5, 4, 6, 9, 8, 7});
  assert(ltl::equal(v, std::vector{4, 5, 6, 7, 8, 9}));

  assert(3 == ltl::min_element_value(odds));
  assert(11 == ltl::max_element_value(odds));
  assert((std::pair{3, 11} == ltl::minmax_element_value(odds)));
  assert(0 == ltl::min_element_value(std::vector<int>{}));
  assert(0 == ltl::max_element_value(std::vector<int>{}));
}

void test_find_range() {
  const std::array v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  {
    auto find = ltl::find(v, 2);
    auto notFind = ltl::find(v, 25);

    assert(find);
    assert(!notFind);
    assert(**find == 2);
    assert(*find == v.begin() + 2);
  }

  {
    auto find = ltl::find_value(v, 2);
    auto notFind = ltl::find_value(v, 25);

    assert(find);
    assert(!notFind);
    assert(*find == 2);
  }

  {
    auto find = ltl::find_ptr(v, 9);
    auto notFind = ltl::find_ptr(v, 11);

    assert(find == &v[9]);
    assert(notFind == nullptr);
  }

  {
    const std::array toFind = {3, 4, 5, 6};
    const std::array toNotFind = {3, 4, 5, 7};

    auto find = ltl::find_end(v, toFind);
    auto notFind = ltl::find_end(v, toNotFind);
    auto findOneOf = ltl::find_first_of(v, toFind);

    assert(find);
    assert(!notFind);
    assert(findOneOf);
    assert(*find == v.begin() + 3);
    assert(*findOneOf == v.begin() + 3);
  }
}

template <typename T, requires_f(ltl::IsFloatingPoint<T>)> constexpr auto f(T) {
  return 0;
}

template <typename T, requires_f(ltl::IsIntegral<T>)> constexpr auto f(T) {
  return 1;
}

void test_concept() {
  static_assert(f(5) == 1);
  static_assert(f(5.0) == 0);
  static_assert(f(5.0f) == 0);
  static_assert(f(5u) == 1);
  static_assert(f(5ull) == 1);
}

void test_optional() {
  using namespace ltl;
  std::optional<int> a(5), b;
  auto times_3 = map(_((x), x * 3));
  auto plus_1 = map(_((x), x + 1));
  auto identity = map([](auto x) -> std::optional<int> { return x; });
  assert((a | times_3) == 15);
  assert((b | times_3) == std::nullopt);

  assert((a | times_3 | plus_1) == 16);
  assert((b | times_3 | plus_1) == std::nullopt);
  assert((a >> identity) == 5);
}

void test_range_view() {
  using namespace ltl;
  const std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto array_view1 = Range{array};
  auto array_view2 = Range{array.begin(), array.end()};
  assert(!array_view1.empty());
  assert(array_view2.size() == array_view1.size() &&
         array.size() == array_view1.size());
  assert(&array.front() == &array_view1.front() &&
         &array.back() == &array_view2.back());
}

void test_filter() {
  using namespace ltl;
  const std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  const std::array oddsArray = {1, 3, 5, 7, 9, 11};
  const std::array evenArray = {0, 2, 4, 6, 8, 10, 12};
  const std::array oddsSuperiorThan5Array = {7, 9, 11};
  auto isOdd = _((x), x % 2);
  auto isEven = _((x), x % 2 == 0);
  auto superiorThan = [](auto x) { return [x](auto y) { return y > x; }; };
  assert(ltl::equal(array | filter(isOdd), oddsArray));
  assert(ltl::equal(array | filter(isEven), evenArray));
  assert(ltl::equal(array | filter(isOdd) | filter(superiorThan(5)),
                    oddsSuperiorThan5Array));

  auto odds = array | filter(isOdd);
  auto evens = array | filter(isEven);
  auto oddSuperiorThan5Filter = filter(isOdd) | filter(superiorThan(5));
  auto oddsSuperiorThan5 = array | oddSuperiorThan5Filter;
  assert(odds.size() == 6 && evens.size() == 7 &&
         oddsSuperiorThan5.size() == 3);
  auto [min, max] = ltl::minmax_element(oddsSuperiorThan5);
  assert(*min == 7 && *max == 11);
  assert(&*min == &array[7] && max.operator->().operator->() == &array[11]);
  assert((min + 2) == max);
  assert((max + (-2)) == min);
  assert((max - 2) == min);
  assert((min - (-2)) == max);
}

void test_map() {
  using namespace ltl;
  std::array times2Array = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24};
  std::array times8Array = {0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96};
  std::array oddTimes2 = {2, 6, 10, 14, 18, 22};

  std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  auto times = [](auto x) { return [x](auto y) { return y * x; }; };
  assert(ltl::equal(array | map(times(2)), times2Array));
  assert(ltl::equal(array | map(times(8)), times8Array));
  assert(ltl::equal(array | (map(times(2)) | map(times(2)) | map(times(2))),
                    times8Array));
  assert(
      ltl::equal(array | (filter(_((x), x % 2)) | map(times(2))), oddTimes2));
}

void test_to() {
  using namespace ltl;
  using namespace std::literals;
  const std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  std::array oddTimes2Array = {2, 6, 10, 14, 18, 22};
  std::array strs = {"2"s, "6"s, "10"s, "14"s, "18"s, "22"s};
  auto oddTimes2 = (array | filter(_((x), x % 2)) | map(_((x), x * 2)));
  assert(ltl::equal(oddTimes2, oddTimes2Array));

  auto oddTimes2List = oddTimes2 | map(_((x), std::to_string(x))) | to_list;
  auto oddTimes2Deque = oddTimes2 | map(_((x), std::to_string(x))) | to_deque;
  auto oddTimes2Vector = oddTimes2 | map(_((x), std::to_string(x))) | to_vector;

  std::vector<std::string> oddTimes2Vector2 =
      oddTimes2 | map(_((x), std::to_string(x)));

  std::deque<std::string> oddTimes2Deque2 =
      oddTimes2 | map(_((x), std::to_string(x)));

  std::list<std::string> oddTimes2List2 =
      oddTimes2 | map(_((x), std::to_string(x)));

  assert(ltl::equal(oddTimes2Deque, strs));
  assert(ltl::equal(oddTimes2List, strs));
  assert(ltl::equal(oddTimes2Vector, strs));

  assert(ltl::equal(oddTimes2Vector2, strs));
  assert(ltl::equal(oddTimes2Deque2, strs));
  assert(ltl::equal(oddTimes2List2, strs));

  typed_static_assert(type_v<std::vector<std::string>> ==
                      type_from(oddTimes2Vector));
  typed_static_assert(type_v<std::deque<std::string>> ==
                      type_from(oddTimes2Deque));
  typed_static_assert(type_v<std::list<std::string>> ==
                      type_from(oddTimes2List));
}

void test_integer_list() {
  std::array values = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  assert(ltl::equal(ltl::valueRange(5) | ltl::take_n(10), values));
  assert(ltl::equal(ltl::valueRange(5, 10), values | ltl::take_n(5)));
}

void test_zip() {
  using namespace std::literals;
  using ltl::tuple_t;
  const std::array strings = {"1"s, "2"s, "3"s, "4"s, "5"s};
  const std::array strings2 = {"One"s, "Two"s, "Three"s, "Four"s, "Five"s};
  std::array integers = {1, 2, 3, 4, 5};

  std::size_t index = 0;
  for (auto [i, s] : ltl::enumerate(strings)) {
    assert(i == index);
    assert(&s == &strings[i]);
    ++index;
  }

  for (auto [index, t] :
       ltl::enumerate(ltl::zip(integers, strings, strings2))) {
    auto [i, s, s2] = t;
    assert(&i == &integers[index]);
    assert(&s == &strings[index]);
    assert(&s2 == &strings2[index]);
  }
}

void test_default_view() {
  using namespace std::literals;
  std::array<std::optional<int>, 5> array{};
  array[1] = 8;
  array[4] = 14;

  std::array<std::size_t, 2> indices{1, 4};
  for (auto [i, e] :
       ltl::enumerate(array | ltl::remove_null() | ltl::dereference())) {
    assert(&e == std::addressof(*array[indices[i]]));
  }

  std::array<ltl::tuple_t<int, double, std::string>, 3> tuples{
      ltl::tuple_t{8, 80.0, "80"s}, ltl::tuple_t{1, 1.0, "1"s},
      ltl::tuple_t{3, 3.0, "3"s}};

  for (auto [i, d] : ltl::enumerate(tuples | ltl::get(1_n))) {
    assert(&d == &tuples[i][1_n]);
  }

  for (auto [index, t] : ltl::enumerate(tuples | ltl::get(2_n, 0_n))) {
    auto [s, i] = t;
    assert(&s == &tuples[index][2_n]);
    assert(&i == &tuples[index][0_n]);
    i++;
  }
}

void test_variant_utils() {
  std::variant<int, double> variant = 5;
  ltl::match(
      variant, [](int) { assert(true); }, [](double) { assert(false); });
  variant = 5.0;
  ltl::match(
      variant, [](double) { assert(true); }, [](int) { assert(false); });

  auto result = ltl::match_result(
      variant, [](int x) { return static_cast<double>(x); },
      [](double x) { return static_cast<int>(x); });

  static_assert(type_from(result) == ltl::type_v<std::variant<double, int>>);
  static_assert(type_from(result) != ltl::type_v<std::variant<int, double>>);
  ltl::match(
      result, [](int) { assert(true); }, [](double) { assert(false); });
  result = 5.0;
  ltl::match(
      result, [](double) { assert(true); }, [](int) { assert(false); });

  {
    std::variant<int, double, std::string> variant;
    auto ok1 = [](auto &&) {};
    auto ok2 = ltl::overloader{[](int) {}, [](double) {}, [](std::string) {}};
    auto ok3 = ltl::overloader{[](int) {}, [](double) {}, [](std::string &) {}};
    auto ok4 =
        ltl::overloader{[](int) {}, [](double) {}, [](const std::string &) {}};
    auto bad1 =
        ltl::overloader{[](int &&) {}, [](double &&) {}, [](std::string &&) {}};
    auto bad2 = [](int) {};
    typed_static_assert(ltl::is_callable_from(ok1, variant));
    typed_static_assert(ltl::is_callable_from(ok2, variant));
    typed_static_assert(ltl::is_callable_from(ok3, variant));
    typed_static_assert(ltl::is_callable_from(ok4, variant));
    typed_static_assert(!ltl::is_callable_from(bad1, variant));
    typed_static_assert(!ltl::is_callable_from(bad2, variant));

    typed_static_assert(!ltl::is_callable_from(ok3, std::as_const(variant)));
    typed_static_assert(ltl::is_callable_from(ok4, std::as_const(variant)));

    typed_static_assert(!ltl::is_callable_from(ok3, std::move(variant)));
    typed_static_assert(ltl::is_callable_from(ok4, std::move(variant)));
    typed_static_assert(ltl::is_callable_from(bad1, std::move(variant)));
  }
}

void test_functional() {
  using namespace std::literals;

  auto factorial =
      ltl::fix{[](auto f, auto x) -> int { return x ? x * f(x - 1) : 1; }};
  static_assert(factorial(1) == 1);
  static_assert(factorial(2) == 2);
  static_assert(factorial(3) == 6);
  static_assert(factorial(4) == 24);
  static_assert(factorial(5) == 120);

  auto divisor = _((w, x, y, z), w / x / y / z);

  static_assert(divisor(1000, 10, 10, 10) == 1);

  assert(ltl::report_call(divisor, 1000, 10, 5, 5)() == 4);
  assert(ltl::report_call(divisor, 1000, 10, 5)(5) == 4);
  assert(ltl::report_call(divisor, 1000, 10)(5, 5) == 4);
  assert(ltl::report_call(divisor, 1000)(10, 5, 5) == 4);
  assert(ltl::report_call(divisor)(1000, 10, 5, 5) == 4);

  assert(ltl::curry(divisor, 1000, 10, 5, 5) == 4);
  assert(ltl::curry(divisor, 1000, 10, 5)(5) == 4);
  assert(ltl::curry(divisor, 1000, 10)(5, 5) == 4);
  assert(ltl::curry(divisor)(1000, 10, 5, 5) == 4);
  assert(ltl::curry(divisor)(1000)(10)(5)(5) == 4);

  struct test {
    int sum(int a, int b, int c) { return a + b + c; }
  };

  test t;
  assert(ltl::curry(&test::sum, t, 1, 2, 3) == 6);
  assert(ltl::curry(&test::sum)(t, 1, 2, 3) == 6);
  assert(ltl::curry(&test::sum, t, 1)(2, 3) == 6);
  assert(ltl::curry(&test::sum)(t)(1)(2, 3) == 6);
  assert(ltl::curry(&test::sum)(t)(1)(2)(3) == 6);

  assert(ltl::curry(&test::sum)(test{})(1)(2)(3) == 6);

  auto a = "One"s;
  auto b = "Two"s;

  auto curry = ltl::curry([](auto a, auto b, auto c, auto d, auto e, auto f) {
    return a + b + c + d + e + f;
  });
  auto one = curry(a);
  auto four = one(std::move(b), "Three"s, "Four");
  auto end = four(a, b);
  assert(end == "OneTwoThreeFourOne");
}

/// Issues are:
/// contains algos for transformed iterator
/// for_each for tuple called instead of algos
/// reverse iterator
/// std invoke
void test_fix_issue_1() {
  std::array a = {0, 1, 2, 3, 4};
  auto square = _((x), x * x);
  assert(ltl::contains(a | ltl::map(square), 9));
  assert(!ltl::contains(a | ltl::map(square), 2));

  ltl::for_each(a, [](auto x) {
    assert(x == 0 || x == 1 || x == 2 || x == 3 || x == 4);
  });

  assert(ltl::equal(a | ltl::reversed | ltl::map(square),
                    std::array{16, 9, 4, 1, 0}));

  struct obj {
    bool isSet() const { return m_isSet; }
    bool m_isSet;
  };

  std::array b = {obj{false}, obj{true}, obj{false}};
  std::array c = {obj{false}, obj{false}, obj{false}};
  std::array d = {obj{true}, obj{true}, obj{true}};

  auto isSet = ltl::map(&obj::isSet);
  auto isNotSet = ltl::map(ltl::not_(&obj::isSet));
  auto id = [](const auto &x) { return x; };
  assert(ltl::any_of(b | isSet, id));
  assert(ltl::none_of(c | isSet, id));
  assert(ltl::all_of(d | isSet, id));
  assert(ltl::all_of(c | isNotSet, id));
  assert(ltl::none_of(d | isNotSet, id));
}

void test_associative_map() {
  std::unordered_map<std::string, std::string> dict = {
      {"a", "A"}, {"b", "B"}, {"c", "C"}, {"d", "D"}, {"e", "E"}, {"f", "F"}};

  assert(ltl::contains_map(dict, "a"));
  assert(ltl::contains_map(dict, "d"));
  assert(!ltl::contains_map(dict, "g"));
  assert(ltl::find_map_value(dict, "e") == "E");
  assert(ltl::find_map_ptr(dict, "f") == &dict["f"]);
  assert(ltl::find_map_value(dict, "g") == std::nullopt);
  assert(ltl::take_map(dict, "f") == "F");
  assert(ltl::find_map_ptr(dict, "f") == nullptr);
}

void test_composition() {
  auto plus_3 = _((x), x + 3);
  auto mul_5 = _((x), x * 5);
  auto minus_1 = _((x), x - 1);

  constexpr auto pMm = ltl::compose(plus_3, mul_5, minus_1);
  constexpr auto mpM = ltl::compose(minus_1, plus_3, mul_5);
  constexpr auto mMp = ltl::compose(minus_1, mul_5, plus_3);

  static_assert(pMm(10) == 64);
  static_assert(mpM(10) == 60);
  static_assert(mMp(10) == 48);
}

void test_join() {
  using namespace ltl;
  struct Test {
    Test(int n) {
      for (int i = 0; i < n; ++i)
        v.push_back(i);
    }
    std::vector<int> v;
  };
  std::array array = {0, 1, 2, 3, 4, 5};
  auto to_range = [](auto n) { return valueRange(0, n); };
  auto arrayRange = array >> map(to_range);
  assert(equal(arrayRange,
               std::array{0, 0, 1, 0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4}));

  std::array array2 = {Test{0}, Test{1}, Test{2}, Test{3}, Test{4}, Test{5}};
  auto to_vector_ref = [](auto &x) -> std::vector<int> & { return x.v; };
  auto to_vector = [](auto &x) { return x.v; };
  auto to_ptr = [](auto &x) { return std::addressof(x); };
  std::array array2ptr = {&array2[1].v[0], &array2[2].v[0], &array2[2].v[1],
                          &array2[3].v[0], &array2[3].v[1], &array2[3].v[2],
                          &array2[4].v[0], &array2[4].v[1], &array2[4].v[2],
                          &array2[4].v[3], &array2[5].v[0], &array2[5].v[1],
                          &array2[5].v[2], &array2[5].v[3], &array2[5].v[4]};

  assert(equal(arrayRange, array2 >> map(to_vector)));
  assert(equal(arrayRange, array2 >> map(to_vector_ref)));
  assert(equal(array2ptr, array2 >> map(to_vector_ref) | map(to_ptr)));
}

void test_and_or() {
  auto is_multiple_of = [](auto x) {
    return [x](auto y) { return y % x == 0; };
  };

  static_assert(ltl::or_(is_multiple_of(3), is_multiple_of(5))(9));
  static_assert(!ltl::and_(is_multiple_of(3), is_multiple_of(5))(9));
  static_assert(ltl::and_(is_multiple_of(3), is_multiple_of(5))(15));
}

void test_optional_type() {
  auto tuple = ltl::tuple_t<int, double, float, int *>{};
  auto tuple2 = tuple + ltl::tuple_t{(char *)nullptr};
  auto empty = ltl::find_type(tuple2, ltl::type_v<double *>);

  static_assert(!empty.has_value);
  static_assert(ltl::find_type(tuple2, ltl::type_v<char *>).has_value);
  static_assert(*ltl::find_type(tuple2, ltl::type_v<char *>) == 4_n);
  static_assert(ltl::find_if_type(tuple2, ltl::is_pointer).has_value);
  static_assert(*ltl::find_if_type(tuple2, ltl::is_pointer) == 3_n);
  static_assert(
      !ltl::find_if_type(tuple2, ltl::is_type(ltl::type_v<char>)).has_value);

  auto two = ltl::find_type(tuple2, ltl::type_v<float>);

  constexpr auto plus_3 = ltl::map([](auto n) { return n + 3_n; });

  auto five = two | plus_3;
  auto empty2 = empty | plus_3;

  static_assert(five.has_value);
  static_assert(*five == 5_n);
  static_assert(empty2.has_value == false_v);

  constexpr auto plus_3_if_sup_4 = ltl::map([](auto n) {
    if constexpr (n > 4_n) {
      return ltl::optional_type{n + 3_n};
    } else {
      return ltl::nullopt_type;
    }
  });

  auto empty3 = two >> plus_3_if_sup_4;
  auto eight = five >> plus_3_if_sup_4;

  static_assert(!empty3.has_value);
  static_assert(eight.has_value);
  static_assert(*eight == 8_n);
}

void test_condition() {
  static_assert(ltl::AllOf{5, 3, 4} > 2);
  static_assert(ltl::AllOf{5, 3, 4} < 10);
  static_assert(!(ltl::AllOf{5, 3, 4} <= 4));
  static_assert(!(ltl::AllOf{5, 3, 4} >= 4));
  static_assert((ltl::AllOf{5, 3, 4} >= 3));
  static_assert((ltl::AllOf{5, 3, 4} <= 6));
  static_assert(ltl::AllOf{5, 5, 5} == 5);
  static_assert(!(ltl::AllOf{7, 8, 10} == 9));
  static_assert(ltl::AllOf{7, 8, 10} != 9);
  static_assert(!(ltl::AllOf{7, 8, 10} != 8));

  static_assert(ltl::AnyOf{8, 9, 10} < 19);
  static_assert(!(ltl::AnyOf{8, 9, 10} < 7));
  static_assert(ltl::AnyOf{8, 9, 10} > 9);
  static_assert(!(ltl::AnyOf{8, 9, 10} > 12));
  static_assert(ltl::AnyOf{9, 9, 18} == 9);
  static_assert(!(ltl::AnyOf{9, 9, 18} == 10));
  static_assert(ltl::AnyOf{9, 9, 18} != 9);
  static_assert(!(ltl::AnyOf{9, 9, 9} != 9));
  static_assert(ltl::AnyOf{8, 9, 10} >= 10);
  static_assert(!(ltl::AnyOf{8, 9, 10} >= 15));
  static_assert(ltl::AnyOf{8, 9, 10} <= 10);
  static_assert(!(ltl::AnyOf{8, 9, 10} <= 5));

  static_assert(ltl::NoneOf{8, 9, 10} < 8);
  static_assert(!(ltl::NoneOf{8, 9, 10} < 9));
  static_assert(ltl::NoneOf{8, 9, 10} > 10);
  static_assert(!(ltl::NoneOf{8, 9, 10} > 9));
  static_assert(ltl::NoneOf{9, 9, 18} == 8);
  static_assert(!(ltl::NoneOf{9, 9, 18} == 18));
  static_assert(ltl::NoneOf{9, 9, 9} != 9);
  static_assert(!(ltl::NoneOf{9, 9, 18} != 9));
  static_assert(ltl::NoneOf{8, 9, 10} >= 12);
  static_assert(!(ltl::NoneOf{8, 9, 10} >= 10));
  static_assert(ltl::NoneOf{8, 9, 10} <= 6);
  static_assert(!(ltl::NoneOf{8, 9, 10} <= 15));
}

void test_curry_metaprogramming() {
  constexpr ltl::type_list_t<int, double, char, int *, double *> list;

  constexpr ltl::type_list_t<int *, double *, char *> list2;
  constexpr ltl::type_list_t<double *, int *, char, double> list3;

  static_assert(
      ltl::all_of_type(list3, ltl::curry(lift(ltl::contains_type))(list)));
  static_assert(
      !ltl::all_of_type(list2, ltl::curry(lift(ltl::contains_type), list)));
}

int main() {
  bool_test();
  type_test();
  number_test();

  tuple_test();
  tuple_test_algo();
  constexpr_tuple_test();
  tuple_reference_test();
  push_pop_test();

  test_is_valid();
  test_trait();
  test_strong_type();

  test_qualifier();

  test_algos();
  test_find_range();

  test_concept();
  test_optional();

  test_range_view();
  test_filter();
  test_map();
  test_to();

  test_integer_list();
  test_zip();

  test_default_view();
  test_variant_utils();

  test_functional();
  test_fix_issue_1();
  test_associative_map();

  test_composition();
  test_join();

  test_optional_type();
  test_condition();
  test_curry_metaprogramming();

  return 0;
}
