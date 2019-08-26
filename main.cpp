#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "ltl/StrongType.h"
#include "ltl/Tuple.h"
#include "ltl/algos.h"
#include "ltl/ltl.h"
#include "ltl/operator.h"
#include "ltl/range.h"
#include "ltl/stream.h"

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
}

void type_test() {
  static_assert(ltl::type_v<int> == ltl::type_v<int>);
  static_assert(ltl::type_v<int> != ltl::type_v<double>);
  static_assert(!(ltl::type_v<int> == ltl::type_v<double>));
  static_assert(ltl::type_v<decltype(true_v == true_v)> ==
                ltl::type_v<ltl::true_t>);
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

  static_assert(ltl::type_v<std::decay_t<decltype(tuple)>> ==
                ltl::type_v<ltl::tuple_t<int, double>>);
  static_assert(apply(tuple, [](auto a, auto b) { return a + b; }) == 8.0);

  static_assert(tuple.get(0_n) == 5 && tuple.get(1_n) == 3.0);
  static_assert(ltl::tuple_t{5, 3.0}.get(0_n) == 5 &&
                ltl::tuple_t{5, 3.0}.get(1_n) == 3.0);

  static_assert(tuple[0_n] == 5 && tuple[1_n] == 3.0);
  static_assert(ltl::tuple_t{5, 3.0}[0_n] == 5 &&
                ltl::tuple_t{5, 3.0}[1_n] == 3.0);

  static_assert(tuple == ltl::tuple_t<int, double>{5, 3.0});
  static_assert(tuple != ltl::tuple_t<int, double>{5, 3.1});

  static_assert(tuple != ltl::type_list_v<int, double>);
  static_assert(tuple != ltl::number_list_v<5, 3>);
  static_assert(tuple != ltl::bool_list_v<false, true>);

  static_assert(ltl::type_list_v<int, double, char> !=
                ltl::type_list_v<int, double>);

  static_assert(
      ltl::type_list_v<int, double, char> ==
      ltl::type_list_v<double>.push_back(ltl::type_v<char>).push_front(ltl::type_v<int>));

  static_assert(ltl::type_list_v<int, double, double, char, double> !=
                ltl::type_list_v<int, double, char>);

  static_assert(
                ltl::type_list_v<float, int, double, double, char,
                double>.pop_front().pop_back().pop_back().pop_back().push_back(ltl::type_v<char>) ==
                ltl::type_list_v<int, double, char>);

  constexpr auto number_list = ltl::number_list_v<2, 3, 4>;

  static_assert(number_list.push_back(5_n).push_back(6_n) ==
                ltl::number_list_v<2, 3, 4, 5, 6>);
  static_assert(number_list.push_front(1_n).push_front(0_n) ==
                ltl::number_list_v<0, 1, 2, 3, 4>);

  static_assert(number_list.pop_back().pop_back() == ltl::number_list_v<2>);
  static_assert(number_list.pop_front().pop_front() == ltl::number_list_v<4>);
  static_assert(build_index_sequence(5_n) == ltl::number_list_v<0, 1, 2, 3, 4>);
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
  auto [a7, b7, c7, d7] = std::move(tuple);
  auto &&[a8, b8, c8, d8] = std::move(tuple);
  assert(&a2 == &tuple[0_n]);
  assert(&b2 == &tuple[1_n]);
  assert(&c2 == &tuple[2_n]);
  assert(&d2 == &tuple[3_n]);

  assert(&a5 == &tuple[0_n]);
  assert(&b5 == &tuple[1_n]);
  assert(&c5 == &tuple[2_n]);
  assert(&d5 == &tuple[3_n]);

  assert(&a6 == &tuple[0_n]);
  assert(&b6 == &tuple[1_n]);
  assert(&c6 == &tuple[2_n]);
  assert(&d6 == &tuple[3_n]);

  assert(&a8 == &tuple[0_n]);
  assert(&b8 == &tuple[1_n]);
  assert(&c8 == &tuple[2_n]);
  assert(&d8 == &tuple[3_n]);
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

    typed_static_assert(ltl::find_type(tuple, ltl::type_v<double>) == 1_n);
    typed_static_assert(ltl::find_type(tuple, ltl::type_v<int>) == 3_n);
  }

  {
    ltl::type_list_t<int, double, int, int> tuple;
    typed_static_assert(ltl::contains_type(tuple, ltl::type_v<int>));
    typed_static_assert(!ltl::contains_type(tuple, ltl::type_v<char>));
    typed_static_assert(ltl::count_type(tuple, ltl::type_v<int>) == 3_n);
    typed_static_assert(ltl::find_type(tuple, ltl::type_v<int>) == 0_n);
    typed_static_assert(ltl::find_type(tuple, ltl::type_v<int>, 0_n + 1_n) ==
                        2_n);
    typed_static_assert(ltl::find_type(tuple, ltl::type_v<double>) == 1_n);
  }

  {
    ltl::type_list_t<int, double *, int *, int, char *> tuple;
    typed_static_assert(ltl::contains_if_type(tuple, ltl::is_pointer));
    typed_static_assert(ltl::count_if_type(tuple, ltl::is_pointer) == 3_n);
    typed_static_assert(ltl::find_if_type(tuple, ltl::is_pointer) == 1_n);
    typed_static_assert(ltl::find_if_type(tuple, ltl::is_pointer, 2_n) == 2_n);
  }

  {
    ltl::type_list_t<int, int, unsigned int, char> tuple1;
    typed_static_assert(ltl::all_of_type(tuple1, ltl::is_integral));
    typed_static_assert(ltl::none_of_type(tuple1, ltl::is_floating_point));
    typed_static_assert(ltl::any_of_type(tuple1, ltl::is_unsigned));
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
  using base = decltype(ltl::tuple_t{a, std::ref(a), 0});

  using pushedBackRef = decltype(std::declval<base>().push_back(std::ref(a)));
  using pushedBack = decltype(std::declval<base>().push_back(a));
  using pushedFrontRef = decltype(std::declval<base>().push_front(std::ref(a)));
  using pushedFront = decltype(std::declval<base>().push_front(a));

  using popBack = decltype(std::declval<base>().pop_back());
  using popFront = decltype(std::declval<base>().pop_front());

  static_assert(ltl::type_v<base> ==
                ltl::type_v<ltl::tuple_t<int, int &, int>>);

  static_assert(ltl::type_v<pushedBackRef> ==
                ltl::type_v<ltl::tuple_t<int, int &, int, int &>>);
  static_assert(ltl::type_v<pushedBack> ==
                ltl::type_v<ltl::tuple_t<int, int &, int, int>>);
  static_assert(ltl::type_v<pushedFrontRef> ==
                ltl::type_v<ltl::tuple_t<int &, int, int &, int>>);
  static_assert(ltl::type_v<pushedFront> ==
                ltl::type_v<ltl::tuple_t<int, int, int &, int>>);

  static_assert(ltl::type_v<popBack> == ltl::type_v<ltl::tuple_t<int, int &>>);
  static_assert(ltl::type_v<popFront> == ltl::type_v<ltl::tuple_t<int &, int>>);

  ltl::tuple_t testOpBracket{a, std::ref(a), 0};
  using TestOpBracket = decltype(testOpBracket);

  static_assert(ltl::type_v<TestOpBracket> ==
                ltl::type_v<ltl::tuple_t<int, int &, int>>);
  static_assert(ltl::type_v<decltype(testOpBracket[1_n])> ==
                ltl::type_v<int &>);
  static_assert(ltl::type_v<decltype(testOpBracket[0_n])> ==
                ltl::type_v<int &>);
  static_assert(ltl::type_v<decltype(testOpBracket[2_n])> ==
                ltl::type_v<int &>);

  static_assert(ltl::type_v<decltype(std::declval<TestOpBracket>()[1_n])> ==
                ltl::type_v<int &>);
  static_assert(ltl::type_v<decltype(std::declval<TestOpBracket>()[0_n])> ==
                ltl::type_v<int &&>);
  static_assert(ltl::type_v<decltype(std::declval<TestOpBracket>()[2_n])> ==
                ltl::type_v<int &&>);
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
  static_assert(ltl::is_null_pointer(ltl::type_v<decltype(nullptr)>));
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

  {
    int lvalue;
    const int clvalue = 0;
    int carray[5]{};
    std::array<int, 5> array{};
    std::vector<double> array2{};
    std::optional<int> opt;
    static_assert(!ltl::is_rvalue_reference(lvalue));
    static_assert(ltl::is_rvalue_reference(std::move(lvalue)));
    static_assert(ltl::is_rvalue_reference(5));
    static_assert(ltl::is_const(clvalue));
    static_assert(ltl::is_array(carray));
    static_assert(ltl::is_optional(opt));
    static_assert(ltl::is_iterable(carray));
    static_assert(!ltl::is_iterable(opt));
    static_assert(!ltl::is_optional(carray));
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
  static_assert(ltl::type_v<decltype(oneKilometer + oneKilometerInMeter)> ==
                ltl::type_v<Km>);

  // ratio<1000, 1000> is not the same type as ratio<1>, but it is equivalent
  static_assert(ltl::type_v<ltl::multiple_of<Km, std::ratio<1, 1000>>> ==
                ltl::type_v<ltl::multiple_of<Meter, std::ratio<1000, 1000>>>);

  std::cout << oneKilometer << "km = " << oneKilometerInMeter
            << "m = " << oneKilometerInDecimeter << "dm" << std::endl;

  oneKilometerInDecimeter += Meter{30.0f} + Km{1.0f};
  std::cout << "1km + 30m + 1km = " << oneKilometerInDecimeter << "dm"
            << std::endl;

  constexpr radians rad{pi};
  constexpr degrees deg(rad);
  static_assert(rad == deg);
  static_assert(rad.get() == deg.get() * pi / 180.0f);
  static_assert(deg.get() == rad.get() * 180.0f / pi);
}

void test_range() {
  std::array<int, 5> odds = {3, 5, 7, 9, 11};
  auto areEvens = [](auto x) { return (x & 1) == 0; };
  auto areOdds = [](auto x) { return (x & 1) != 0; };
  auto isSuperiorTo = [](auto n) { return [n](auto x) { return x > n; }; };

  assert(ltl::all_of(odds, areOdds));
  assert(ltl::any_of(odds, isSuperiorTo(10)));
  assert(ltl::none_of(odds, areEvens));
  assert(ltl::count(odds, 5) == 1);
  assert(ltl::count(odds, 1) == 0);
  assert(ltl::count_if(odds, isSuperiorTo(4)) == 4);
  assert(ltl::find_if(odds, isSuperiorTo(10)) == odds.begin() + 4);
  assert(ltl::accumulate(odds, 0) == 3 + 5 + 7 + 9 + 11);

  std::array<int, 5> reverseOdds = {11, 9, 7, 5, 3};
  assert(!ltl::equal(reverseOdds, odds));
  ltl::reverse(odds);
  assert(ltl::equal(reverseOdds, odds));

  auto &oddsRef = ltl::sort(odds);
  assert(&oddsRef == &odds);
  assert(ltl::equal(oddsRef, std::array<int, 5>{3, 5, 7, 9, 11}));

  auto v = ltl::sort(std::vector{5, 4, 6, 9, 8, 7});
  assert(ltl::equal(v, std::vector{4, 5, 6, 7, 8, 9}));
}

void test_find_range() {
  const std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  {
    auto notFind = ltl::find(v, 25);
    auto find = ltl::find(v, 2);

    assert(!notFind);
    assert(find);
    assert(*find == v.begin() + 2);
    assert(**find == 2);
  }

  {
    auto notFind = ltl::find_value(v, 25);
    auto find = ltl::find_value(v, 2);
    assert(!notFind);
    assert(find);
    assert(*find == 2);
  }

  {
    const std::vector<int> toFind = {3, 4, 5, 6};
    const std::vector<int> toNotFind = {3, 4, 5, 7};

    auto notFind = ltl::find_end(v, toNotFind);
    auto find = ltl::find_end(v, toFind);
    auto findOneOf = ltl::find_first_of(v, toFind);

    assert(!notFind);
    assert(find);
    assert(findOneOf);
    assert(*find == v.begin() + 3);
    assert(*findOneOf == v.begin() + 3);
  }
}

template <typename T, requires_f(ltl::IsFloatingPoint<T>)> void f(T) {
  std::cout << "Floating point" << std::endl;
}

template <typename T, requires_f(ltl::IsIntegral<T>)> void f(T) {
  std::cout << "Integral" << std::endl;
}

void test_concept() {
  f(5);
  f(5.0);
  f(5.0f);
  f(5u);
  f(5ull);
}

void test_optional() {
  std::cout << std::endl << "test_optional" << std::endl;
  std::optional<int> a(5), b;
  ltl::cout << a << "\n" << b << "\n";

  auto times_3 = _((x), x * 3);
  auto plus_1 = _((x), x + 1);
  auto identity = [](auto x) -> std::optional<int> { return x; };
  ltl::cout << (a | times_3) << "\n";
  ltl::cout << (b | times_3) << "\n";

  ltl::cout << (a | times_3 | plus_1) << "\n";
  ltl::cout << (b | times_3 | plus_1) << "\n";
  ltl::cout << (a >> identity) << "\n";
}

void test_range_view() {
  using namespace ltl;
  constexpr std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
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
  std::cout << "Filter test: " << std::endl;
  std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  auto isOdd = _((x), x % 2);
  auto isEven = _((x), x % 2 == 0);
  auto superiorThan = [](auto x) { return [x](auto y) { return y > x; }; };
  ltl::cout << "Odds: " << (array | filter(isOdd)) << "\n";
  ltl::cout << "Even: " << (array | filter(isEven)) << "\n";
  ltl::cout << "Odds superior than 5: "
            << (array | filter(isOdd) | filter(superiorThan(5))) << "\n";

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

  std::cout << "Map test: " << std::endl;
  std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  auto times = [](auto x) { return [x](auto y) { return y * x; }; };
  ltl::cout << "times 2: " << (array | map(times(2))) << "\n";
  ltl::cout << "times 8: " << (array | map(times(8))) << "\n";
  ltl::cout << "times 8: "
            << (array | (map(times(2)) | map(times(2)) | map(times(2))))
            << "\n";

  ltl::cout << "odd times 2: "
            << (array | (filter(_((x), x % 2)) | map(times(2)))) << "\n";
}

void test_to() {
  using namespace ltl;
  using namespace std::literals;
  std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  std::array strs = {"2"s, "6"s, "10"s, "14"s, "18"s, "22"s};
  auto oddTimes2 = (array | (filter(_((x), x % 2)) | map(_((x), x * 2))));
  ltl::cout << "odd times 2: " << oddTimes2 << "\n";

  auto oddTimes2List = oddTimes2 | map(_((x), std::to_string(x))) | to_list;
  auto oddTimes2Deque = oddTimes2 | map(_((x), std::to_string(x))) | to_deque;
  auto oddTimes2Vector = oddTimes2 | map(_((x), std::to_string(x))) | to_vector;
  assert(ltl::equal(oddTimes2Deque, strs));
  assert(ltl::equal(oddTimes2List, strs));
  assert(ltl::equal(oddTimes2Vector, strs));
  typed_static_assert(type_v<std::vector<std::string>> ==
                      type_v<decltype(oddTimes2Vector)>);
  typed_static_assert(type_v<std::deque<std::string>> ==
                      type_v<decltype(oddTimes2Deque)>);
  typed_static_assert(type_v<std::list<std::string>> ==
                      type_v<decltype(oddTimes2List)>);
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

  test_range();
  test_find_range();

  test_concept();
  test_optional();

  test_range_view();
  test_filter();
  test_map();
  test_to();

  return 0;
}
