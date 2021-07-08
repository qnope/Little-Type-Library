# Other utilities
## Helper for conditions
Available in `ltl/condition.h` header file, you will find `ltl::AllOf`, `ltl::AnyOf` and `ltl::NoneOf`

These helpers are easy to use:

```cpp
constexpr int a = 8, b = 8, c = 8;

static_assert(AllOf{a, b, c} == 8);
static_assert(AllOf{a, b, c} <= 8);
static_assert(AllOf{true, true, true});
static_assert(AnyOf{false, true, false});
```
All of the operator are available: `<, <=, ==, !=, >=, >`

## Variant Helpers
Available in `ltl/VariantUtils.h` header file, you will find:

  * `is_callable_from(f, variant)`: Function that test if f can handle every types of the variant
  * `match(variant, f...)`: Function that makes easier the use of several lambdas when trying to visit a variant
  * `recursive_wrapper`: Helper for recursive variants
  * `recursive_variant`: A true recursive variant:

```cpp
void test_variant_recursive() {
  using namespace ltl;

  using Variant =
      recursive_variant<int, double, std::string, recursive_wrapper<struct R>>;
  struct R {
    Variant v;
  };

  Variant i = 5;
  Variant d = 5.0;
  Variant s = std::string{"Test"};
  Variant r = 0;
  R firstR{8};
  firstR.v = R{8};
  r = std::move(firstR);

  recursive_visit(overloader{[](auto &&) { assert(false); }, [](int) {}}, i);
  recursive_visit(overloader{[](auto &&) { assert(false); }, [](double) {}}, d);
  recursive_visit(
      overloader{[](auto &&) { assert(false); }, [](std::string) {}}, s);
  recursive_visit(overloader{[](auto &&) { assert(false); },
                             [](R &r) {
                               recursive_visit(
                                   overloader{[](auto &&) { assert(false); },
                                              [](R &r) {
                                                recursive_visit(
                                                    overloader{[](auto &&) {
                                                                 assert(false);
                                                               },
                                                               [](int) {}},
                                                    r.v);
                                              }},
                                   r.v);
                             }},
                  r);
}
```

## Strong types
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

## Streambuf

In `ltl/stream.h`, you will find two `streambuf` class to write or read from an array.

  * `ltl::basic_readonly_streambuf`
  * `ltl::basic_writeonly_streambuf`
