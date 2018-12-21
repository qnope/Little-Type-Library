# Little Type Library

## Introduction
**Little Type Library** is a little metaprogramming library that can be used if you do not want depends on **boost::hana** or equivalent because they are too heavy or whatsoever.

The namespace of thiss library is `namespace ltl`. The library provides several different types as `bool_t`, `number_t`, `type_t` and `tuple_t`

## bool_t

The `bool_t` type is used to have a compile time type that means `true` or `false`. For using these tlypes, there are the following declarations 

```cpp
constexpr ltl::true_t true_v;
constexpr ltl::false_t false_v;
```

## number_t

As for `bool_t`, `number_t` type is used to have a compile type unsigned integer value. You can use it as follow :

```cpp
constexpr ltl::number_t<1> one{};
constexpr auto two = 2_n;
static_assert(3_n == one + two && one < two);
```

## type_t

This type is special. It holds a type to give a type to a function by value instead of by template argument. It is easy to use :

```cpp
constexpr ltl::type_t<int> intType;
constexpr auto doubleType == ltl::type_v<double>;
decltype_t(doubleType) aDoubleValue = 0.5;
static_constexpr(intType != doubleType);
```

## tuple_t
The `tuple_t` is a kind of `std::tuple` but it is easier to use because it does not need to use the `<>` template notation.

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
There are also some helpers if you need them :

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
When you want to check at compile time parameter for a function, it is not possible because `constexpr` function must work in a run-time context. However, since we are using our `false_t` or `true_t`, it is possible to tell the compiler to not call the function that operate the test, but only check its return type. That is why you may need such macro.

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

Without these macros, the following code won't compile because `n1 < n2` must work in run-time context and it is the same for `is_number_t`.

## is_valid
There is another little helper that has been made more for fun than other. The `IS_VALID(variables, expression)` macro.

```cpp
  using namespace std::literals;
  constexpr auto additionnable = IS_VALID((x, y), x + y);

  int integer;
  typed_static_assert(additionnable(integer, 8));
  typed_static_assert(!additionnable(integer, "lol"s));
```

## strong_type
If you never heard about strong types, you better go to see the [Jonathan Boccara's blog](http://fluentcpp.com) before reading the following section. Actually, I was inspired a lot from his articles about strong typing. For people that does not want to read everything, here is a little summary.

Let's say you want to develop a GUI application. You want to create a window with a width of 800, and a height of 600. You will probably write something like: `Window myWindow{800, 600};`. However, how do you know if you are using `800` for the width and not for the height? `strong_type`s solve this first problem. Now you will write something like : `Window myWindow{Width{800}, Height{600}};`. Now there is no shadow of a doubt, and, obviously, you can't convert a `Width` into a `Height`. We add a kind of *type-safety* into our code.

### How to declare a strong type in LTL
There is a type : `strong_type_t<T, Tag, Skills...>` in the `namespace ltl`. You can declare, for example, our prior `Width` and `Height` types as follow :

```cpp
using Width = ltl::strong_type_t<float, struct WidthTag>;
using Height = ltl::strong_type_t<float, struct HeightTag>;
```

You can get the value using the `get` function :
```cpp
Width width{5.0f};
std::cout << width.get() << std::endl;
```

### Add operation to strong types
We saw that `strong_type_t`take `Skills...` template parameter. But how to use them?
As of now, Little Type Library has several skills available :

- Arithmetic ones:
	- `Addable` : `operator +`
	- `Subtractable` : `operator -`
	- `Multipliable` : `operator *`
	- `Dividable` : `operator /`
	- `Moduloable` : `operator %`
- Comparison ones:
	- `GreatherThan` : `operator >`
	- `LessThan` : `operator <`
	- `GreatherThanEqual` : `operator >=`
	- `LessThanEqual` : `operator <=`
    - `EqualityComparable` : `operator==` and `operator!=`
    - `AllComparable` : All of the above
- Stream ones:
    - `OStreamable` : `operator <<(std::ostream&)`

You can do as follow with one or several skills above or your own skills :
```cpp
using Float = ltl::strong_type_t<float, struct FloatTag, ltl::OStreamable>;
Float f{8.0f};
std::cout << f << std::endl;
```
### Write your own skills :
Let's say you want to write a skill `Printable`  that adds the function `print()` to the `strong_type`.
There is two ways. The first one is to use CRTP, the second one is to not use CRTP. Basically, you need CRTP for adding member function, and you do not need it for adding `friend` functions. We will only see how to use CRTP because it is harder than not to use it.

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
Let's say you have a type `Meter`. However, you want `Meter` to be convertible to `Kilometer`. We can define `Kilometer` as a multiple of `Meter` :

```
using Meter = ltl::strong_type_t<float, struct DistanceTag>;
using Kilometer = ltl::multiple_of<Meter, std::ratio<1000>>;
```

### Write your own converter
Imagine you want to convert radians to degrees, or Watts to decibels. It is not a simple ratio, and it is not integer values.
The idea is to write your own convert functions !

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