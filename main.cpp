#include <array>
#include <assert.h>
#include <deque>
#include <functional>
#include <ratio>
#include <set>
#include <string>
#include <vector>

#include "ltl/is_valid.h"
#include "ltl/number_t.h"
#include "ltl/overloader.h"
#include "ltl/range.h"
#include "ltl/tuple.h"
#include "ltl/type_t.h"
#include "ltl/type_traits.h"

#include "ltl/strong_type.h"

#include "ltl/smart_iterator.h"

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

  static_assert(2_n * 3_n == 6_n);
  static_assert(!(3_n * 0_n != 0_n));

  static_assert(1024_n - 25_n == 999_n);
  static_assert(1024_n / 2_n == 512_n);
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
      ltl::type_list_v<float, int, double, double, char, double>.pop_front().pop_back().pop_back().pop_back().push_back(ltl::type_v<char>) ==
      ltl::type_list_v<int, double, char>);

  constexpr auto number_list = ltl::number_list_v<2, 3, 4>;

  static_assert(number_list.push_back(5_n).push_back(6_n) ==
                ltl::number_list_v<2, 3, 4, 5, 6>);
  static_assert(number_list.push_front(1_n).push_front(0_n) ==
                ltl::number_list_v<0, 1, 2, 3, 4>);

  static_assert(number_list.pop_back().pop_back() == ltl::number_list_v<2>);
  static_assert(number_list.pop_front().pop_front() == ltl::number_list_v<4>);
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
  static_assert(ltl::is_class(ltl::type_v<Default>));
  static_assert(ltl::is_lvalue_reference(ltl::type_v<int &>));
  static_assert(ltl::is_rvalue_reference(ltl::type_v<int &&>));
  static_assert(ltl::is_reference(ltl::type_v<int &>));
  static_assert(ltl::is_const(ltl::type_v<const int>));
  static_assert(ltl::is_trivially_destructible(ltl::type_v<Default>));
  static_assert(ltl::extent(ltl::type_v<float[5][3]>, 1_n) == 3_n);
  static_assert(ltl::extent(ltl::type_v<float[5][3]>, 0_n) == 5_n);
  static_assert(ltl::extent(ltl::type_v<float[5][3]>) == 5_n);
  static_assert(ltl::add_pointer(ltl::type_v<int>) == ltl::type_v<int *>);
  static_assert(ltl::is_iterable(ltl::type_v<std::vector<int>>));
  static_assert(!ltl::is_iterable(ltl::type_v<int>));
  static_assert(ltl::is_iterable(ltl::type_v<std::array<int, 1>>));
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

  std::cout << oneKilometer << "km = " << oneKilometerInMeter << "m"
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

  const std::vector<int> vec1 = {10, 9, 5, 45, 98};
  auto vec2 = ltl::sort(vec1);
  assert(ltl::equal(vec2, std::vector<int>{5, 9, 10, 45, 98}));
}

void test_smart_iterator() {
  std::array<int, 5> odds = {3, 5, 7, 9, 11};
  auto isSuperiorTo = [](auto n) { return [n](auto x) { return x > n; }; };
  auto multiplyBy = [](auto n) { return [n](auto x) { return x * n; }; };

  auto superiorThan8 = ltl::filter(isSuperiorTo(8));
  auto multiplyBy2 = ltl::map(multiplyBy(2));

  {
    auto oddsEnumerateRange = ltl::enumerate(odds);
    std::vector oddsEnumerate(oddsEnumerateRange.begin(),
                              oddsEnumerateRange.end());

    assert(oddsEnumerate.size() == 5);
    assert(
        ltl::accumulate(oddsEnumerate, std::size_t{0}, [](auto init, auto t) {
          auto [i, v] = t;
          return init + i;
        }) == 0 + 1 + 2 + 3 + 4);

    assert(ltl::accumulate(oddsEnumerate, 0, [](auto init, auto t) {
             auto [i, v] = t;
             return init + v;
           }) == 3 + 5 + 7 + 9 + 11);
  }

  {
    auto oddsSuperiorThan8Range = superiorThan8(odds);
    std::vector oddsSuperiorThan8(oddsSuperiorThan8Range.begin(),
                                  oddsSuperiorThan8Range.end());
    assert(oddsSuperiorThan8.size() == 2);
    assert(oddsSuperiorThan8[0] == 9 && oddsSuperiorThan8[1] == 11);
  }

  {
    auto oddsMultipliedBy2Range = multiplyBy2(odds);
    std::vector oddsMultipliedBy2(oddsMultipliedBy2Range.begin(),
                                  oddsMultipliedBy2Range.end());

    assert(oddsMultipliedBy2.size() == 5);
    assert(ltl::accumulate(oddsMultipliedBy2, std::size_t{0}) ==
           6 + 10 + 14 + 18 + 22);
  }

  auto superiorThan8AfterMultipliedBy2 = superiorThan8(multiplyBy2(odds));
  for (auto [i, v] : ltl::enumerate(superiorThan8AfterMultipliedBy2)) {
    std::cout << i << ":" << v << std::endl;
  }
}

void test_sorted_iterator() {
  std::vector<int> v1 = {25,  -65, 39,  41,   21, -98, 64, -74,
                         -42, 98,  125, -145, 68, 75,  14, 32};

  std::vector<int> v2;
  std::set<int> set;
  std::deque<int> deque;

  ltl::copy(v1, ltl::sorted_inserter(v2));
  ltl::copy(v1, ltl::sorted_inserter(set));
  ltl::copy(v1, ltl::sorted_inserter(deque));

  ltl::sort(v1);

  assert(ltl::equal(v1, v2));
  assert(ltl::equal(v1, set));
  assert(ltl::equal(v1, deque));
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

int main() {
  using namespace std::literals;
  bool_test();
  type_test();
  number_test();
  tuple_test();
  constexpr_tuple_test();
  tuple_reference_test();
  push_pop_test();

  test_is_valid();
  test_trait();
  test_strong_type();

  test_range();
  test_smart_iterator();
  test_sorted_iterator();
  test_find_range();
  return 0;
}
