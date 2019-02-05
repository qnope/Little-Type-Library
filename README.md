# Little Type Library

## Introduction
**Little Type Library** is a little metaprogramming library that can be used if you do not want to depend on **boost::hana** or equivalent because they are too heavy or for whatever reason.

The namespace of this library is `namespace ltl`. The library provides several different types such as `bool_t`, `number_t`, `type_t` and `tuple_t`

## bool_t

The `bool_t` type is used to have a compile time type that means `true` or `false`. To use these types, use the following declarations:

```cpp
constexpr ltl::true_t true_v;
constexpr ltl::false_t false_v;
```

## number_t

As for `bool_t`, `number_t` type is used to have a compile type unsigned integer value. You can use it as follows:

```cpp
constexpr ltl::number_t<1> one{};
constexpr auto two = 2_n;
static_assert(3_n == one + two && one < two);
```

## type_t

This type is special. It holds a type to give a type to a function by value instead of by template argument. It is easy to use:

```cpp
constexpr ltl::type_t<int> intType;
constexpr auto doubleType == ltl::type_v<double>;
decltype_t(doubleType) aDoubleValue = 0.5;
static_constexpr(intType != doubleType);
```

## tuple_t
The `tuple_t` is a kind of `std::tuple` but it is easier to use because it does not need you to use the `<>` template notation.

```cpp
constexpr ltl::tuple_t tuple{5, 3.0};
constexpr auto sum2 = [](auto a, auto b){return a + b;};
static_assert(apply(tuple, sum2) == 8.0); // notation apply
static_assert(tuple(sum2) == 8.0); // notation parenthesis
constexpr auto printTuple = [](auto p){std::cout << a p;};
for_each(tuple, printTuple);
static_assert(tuple[0_n] == 5);
```
You can also append, prepend, remove values from a tuple through `pop/push_front/back` functions.
There are also some helpers if you need them:

```cpp
// Convenience types
template <typename... Types> using type_list_t = tuple_t<type_t<Types>...>;
template <size_t... Ns> using number_list_t = tuple_t<number_t<Ns>...>;
template <bool... Bs> using bool_list_t = tuple_t<bool_t<Bs>...>;

// Convenience variables
template <typename... Types> constexpr type_list_t<Types...> type_list_v{};
template <size_t... Ns> constexpr number_list_t<Ns...> number_list_v{};
template <bool... Bs> constexpr bool_list_t<Bs...> bool_list_v{};
```

## typed_static_assert and if_constexpr macro
When one wants to check parameters of a function at compile time, it is not possible because `constexpr` function must work in a run-time context. However, since we are using our `false_t` or `true_t`, it is possible to tell the compiler to not call the function that operates the test, but only check its return type. That is why you may need such a macro.

```cpp
#define typed_static_assert(f) static_assert(decltype(f){});
#define if_constexpr(c) if constexpr (decltype(c){})

template <typename N1, typename N2>
[[nodiscard]] constexpr auto build_index_sequence(N1 n1, N2 n2) {
  typed_static_assert(is_number_t(n1));
  typed_static_assert(is_number_t(n2));
  typed_static_assert(n1 < n2);
  return tuple_t<>::build_index_sequence(n1, n2);
}
```

Without these macros, the following code wouldn't compile because `n1 < n2` must work in run-time context and it is the same for `is_number_t`.

## Trait
Some traits are imported from the STL. They have the same name as in the STL, but you can use them through functions.

```cpp
  struct Default {
    Default() = default;
  };

  struct NonDefault {
    NonDefault() = delete;
  };

  static_assert(ltl::is_default_constructible(ltl::type_v<Default>));
  static_assert(!ltl::is_default_constructible(ltl::type_v<NonDefault>));
```

## is_valid
There is another little helper that has mainly been made for fun. The `IS_VALID(variables, expression)` macro.

```cpp
  using namespace std::literals;
  constexpr auto additionnable = IS_VALID((x, y), x + y);

  int integer;
  typed_static_assert(additionnable(integer, 8));
  typed_static_assert(!additionnable(integer, "lol"s));
```

## strong_type
If you have never heard about strong types, you better go to see [Jonathan Boccara's blog](http://fluentcpp.com) before reading the following section. Actually, I was inspired a lot by his articles about strong typing. For people who do not want to read everything, here is a little summary.

Let's say you want to develop a GUI application. You want to create a window with a width of 800, and a height of 600. You will probably write something like: `Window myWindow{800, 600};`. However, how do you know if you are using `800` for the width and not for the height? `strong_type`s solves this first problem. Now you will write something like : `Window myWindow{Width{800}, Height{600}};`. Now there is no shadow of a doubt, and, obviously, you can not convert a `Width` into a `Height`. We add a kind of *type-safety* into our code.

### How to declare a strong type in LTL
There is a type : `strong_type_t<T, Tag, Skills...>` in the `namespace ltl`. You can declare, for example, our prior `Width` and `Height` types as follows:

```cpp
using Width = ltl::strong_type_t<float, struct WidthTag>;
using Height = ltl::strong_type_t<float, struct HeightTag>;
```

You can get the value using the `get` function:
```cpp
Width width{5.0f};
std::cout << width.get() << std::endl;
```

### Add operation to strong types
We saw that `strong_type_t`takes `Skills...` template parameters. But how to use them?
As of now, Little Type Library has several skills available:

- Arithmetic ones:
	- `Addable` : `operator +`
	- `Subtractable` : `operator -`
	- `Multipliable` : `operator *`
	- `Dividable` : `operator /`
	- `Moduloable` : `operator %`
	- `Incrementable` : `operator++`
	- `Decrementable`: `operator--`
- Comparison ones:
	- `GreatherThan` : `operator >`
	- `LessThan` : `operator <`
	- `GreatherThanEqual` : `operator >=`
	- `LessThanEqual` : `operator <=`
    - `EqualityComparable` : `operator==` and `operator!=`
    - `AllComparable` : All of the above
- Stream ones:
    - `OStreamable` : `operator <<(std::ostream&)`

You can do as follows with one or several skills above or your own skills:
```cpp
using Float = ltl::strong_type_t<float, struct FloatTag, ltl::OStreamable>;
Float f{8.0f};
std::cout << f << std::endl;
```
### Write your own skills
Let's say you want to write a skill `Printable`  that adds the function `print()` to the `strong_type`.
There are two ways. The first one is to use CRTP, the second one is not to use CRTP. Basically, you need CRTP to add member functions, and you do not need it to add `friend` functions. We will only see how to use CRTP because it is harder than not to use it.

There is a little helper `class crtp` in the `namespace ltl`.

```cpp
template<typename TheStrongType>
struct Printable : ltl::crtp<TheStrongType, Printable> {
    void print() const {
    	/* this->underlying is a crtp member function that
           basically means static_cast<TheStrongType&>(*this) */
    	std::cout << this->underlying().get() << std::endl;
    }
};
```
Done !

### Convert a meter to a kilometer
Let's say you have a type `Meter`. However, you want `Meter` to be convertible to `Kilometer`. We can define `Kilometer` as a multiple of `Meter`:

```
using Meter = ltl::strong_type_t<float, struct DistanceTag>;
using Kilometer = ltl::multiple_of<Meter, std::ratio<1000>>;
```

### Write your own converter
Imagine you want to convert radians to degrees, or Watts to decibels. It is not a simple ratio, and it is not integer values.
The idea is to write your own convert functions!

```cpp
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
```

## Range
This part of **Little Type Library** is for people who are somewhat _lazy_.
Instead of writing for example 
```cpp
auto v1 = getVector();
std::vector<int> v2;
std::copy(std::begin(v1), std::end(v1), std::back_inserter(v2));
```
One can simply write:
```cpp
auto v1 = getVector();
std::vector<int> v2;
ltl::copy(v1, std::back_inserter(v2));
```
## Smart iterators
If you are familiar with [range-v3](https://github.com/ericniebler/range-v3) library, you may already know what it is about. To put it simply, a smart-iterator is an iterator type that adds some operations to a normal iterator. **Little Type Library** currently provides 4 kinds of smart-iterators.

### enumerate_iterator
This one will make python developers happy, it will let you to write something like:
```cpp
std::vector<int> vector = {5, 4, 3, 1, 8, 9, 10};
for(auto [index, value] : ltl::enumerate(vector)) {
    std::cout << "vector[" << index << "] = " << value << "\n";
}
```
### filter_iterator
If you want to filter your vector with lazy initialization, you can do it with this iterator:
```cpp
auto superiorThan8 = ltl::filter([](auto n){return n > 8;});
// will write only values superior than 8
for(auto v : superiorThan8(vector)) {
	std::cout << v << std::endl;
}
```

### map_iterator
You can also perform transformation through this iterator:
```cpp
auto multiplyBy2 = ltl::map([](auto n){return n * 2;});
// will write values multiplied by 2
for(auto v : multiplyBy2(vector)) {
	std::cout << v << std::endl;
}
```

### sorted_iterator
Let's say you want to insert values into a `std::vector` or `std::deque` in a sorted way. 
You can use use a `sorted_inserter_iterator`:

```cpp
std::vector<int> v1 = {25, -65, 39, 41, 21, -98, 64, -74};
std::vector<int> v2;
ltl::copy(v1, ltl::sorted_inserter(v2));
```
