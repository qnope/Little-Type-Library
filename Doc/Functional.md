# Functional library
It is one of the core library of **LTL**. The idea is to try to write code in a functional style keeping high performances and expressiveness.
## Pipelining operations
Standard algorithms are cool. They are fast, easy to use, expressive, customisable and everything we want.
However, they are not easily composable
Given an array of integers, how to sum the square of the odd values, by using algorithms?

```cpp
using namespace ltl;
std::vector array = {0, 1, 2, 3, 4, 5, 6};
auto is_odd = [](auto x){return x % 2 == 1;};
auto square = [](auto x){return x * x;};
array.erase(remove_if(array, not_(is_odd)), end(array));
transform(array, begin(array), square);
auto result = accumulate(array, 0);
```

In shell, we could do something close to

```sh
commandThatReturnIntList | onlyOdd | square | sum
```
The C++ solution using **LTL** is:
```cpp
auto result = array | filter(is_odd) | map(square) | accumulate(0);
```

Here, the `|` notation works also with rvalue reference. C++20 range does not.

### Proxy iterators
Proxy iterators are an abstraction over iterators. They can iterate only on specific values (filtering), or they can transform the underlying value (mapping). **LTL** provides a lot of different proxy iterators.
#### map iterator
Mapping operation allows us to apply a function over each element of the list.
Map iterator are given by the following C++ function

```cpp
template<typename ...Fs>
constexpr auto map(Fs...) noexcept;
```
`Fs` are composed. Let's say we want to apply twice the `square` function

```cpp
for(auto elem : array | map(square, square)) {
    use(elem);
}
// or
for(auto elem : array | map(square) | map(square)) {
    use(elem);
}
```

**LTL** provides also `transform` that is just an alias to `map`

#### filter iterator
Filtering operation allows us to iterate over a subset of elements which satify a predicate.
Filter iterator are given by the following C++ function
```cpp
template<typename ...Fs>
constexpr auto filter(Fs...) noexcept;
```
`Fs` are composed  and must give out a boolean value. Let's say we want to iterate over the square values that are less than 10.

```cpp
std::array array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
for(auto elem : array | filter(square, less_than(10))) {
    use(elem);
}
// or
for(auto elem : array | map(square) | filter(less_than)) {
    use(elem);
}
```
**LTL** provides also `remove_if`.

#### taker
It can happen that we want to take the 10 first values of a container, or remove the 5 first.
**LTL** provides `take_n` and `drop_n` for such operation

```cpp
auto view = array | drop_n(5) | take(10); // take the sixth to the fiftennth element
```
You can also use `drop_while` or `take_while`.

#### split
Let's have a string:
```cpp
std::string string = "Using LTL is great";
```
We want to split it by spaces.
```python
["Using", "LTL", "is", "great"]
```
`SplitIterator` returns something like `Range<It>` when we dereference it, so, we want to transform this range to a `std::string_view`.
```cpp
auto to_view = [](auto &&r) {return std::string_view{&*r.begin(), r.size()};};
auto splitted = string | split(' ') | map(to_view);
// splitted = ["Using", "LTL", "is", "great"]
```
#### group_by
Let's say you have a Player
```cpp
struct Player {
    Team team;
    std::string name;
};
```
You have a collection of Players sorted by teams. You want to iterate over each teams without copying anything or build a map. **LTL** provides `group_by`.

```cpp
std::vector<Player> all_players;

for(auto [team, players] : all_players | group_by(&Player::team)) {
    std::cout << team.name << " : ";
    for(const auto &player : players) {
        std::cout << player.name << " ";
    }
    std::cout << std::endl;
}
```

#### chunks
Chunks are good for you if you want to have views without more than `n` elements.

```cpp
std::vector<int> values(1000);

for(auto views : values(1000) | chunks(100)) {
    for(auto item : views) { // each views has 100 items

    }
}
```
#### reverse
With `reversed` you can iterate over your arrays or your views in reversed way.
```cpp
auto to_view = [](auto &&r) {return std::string_view{&*r.begin(), r.size()};};
auto splitted = string | split(' ') | reversed | map(to_view);
// splitted = ["great", "is", "LTL", "Using"]
```
#### dereference
If you have an array of smart pointers, like `std::unique_ptr`, or an array of `std::optional` you will not be able to map directly over a member function. One way to address this problem could be to use the **LTL**'s `dereference` function.

```cpp
struct Person {
    std::string name;
};

std::vector<Person> persons;
auto names = persons | map(dereference, &Person::name);
// or
auto names = persons | map(dereference) | map(Person::name);
```
#### values, keys, get
These helpers are useful for _tuple_ based container, like `std::map`, `std::vector<std::pair<std::string, int>>` or `std::vector<tuple_t<int, double, std::string>>`.

```cpp
std::map<std::string, int> map;

for(auto [key, value] : map) {
    std::cout << key << " " << value << std::endl;
}

for(auto key : map | keys()) {
    std::cout << key << std::endl;
}

for(auto value : map | values()) {
    std::cout << value << std::endl;
}

std::vector<tuple_t<int, double, std::string>> vector;

for(auto [i, d, str] : vector) {}
for(auto [i, str] : vector | get(0_n, 2_n)) {}
for(auto [d, str, i] : vector | get(1_n, 2_n, 0_n)) {}
```

#### Transforming to a normal container
Sometimes, it can happen that you need to retrieve the result inside a standard container, or simply return the result of an operation.
There is two ways to convert a `Range` into a standard container
  1. Use the fact that container have a constructor like `container(It begin, It end)` (It is the recommanded way)
  2. Use the `to_vector`, `to_list` and `to_queue` 

```cpp
std::vector<int> vector1 = {0, 1, 2, 3, 4, 5, 6};

// here vector2 = vector3 = [0, 2, 4, 6, 8, 10, 12]
std::vector<int> vector2 = vector1 | map([](auto x){return x * 2;});
auto vector3 = vector1 | map([](auto x){return x * 2;}) | to_vector;
```
If you want to construct a `map` from two `vector`, you can use the fact that map wait for iterators of pair
```cpp
std::vector<std::string> strings;
std::vector<int> ints;

std::map<std::string, int> map = zip(strings, ints) | to_pair;
```

## Actions
Actions are a beautiful way to compose modifying algorithms, or to reduce a range to one value (like a find, or fold left)

There is currently 4 kinds of actions that modify a container:
  1. sort
  2. sort_by
  3. unique
  4. reverse

```cpp
std::vector<int> anArrayOfInt;
anArrayOfInt |= actions::sort; // anArrayOfInt is now sorted
auto reversed = anArrayOfInt | actions::reversed;
anArrayOfInt |= actions::unique; // There is no duplicate 

auto re_sortedArrayWithoutDuplicate = reversed | actions::sort | actions::unique;
```

There is currently 8 non modyfing actions
  1. find
  2. find_value
  3. find_ptr
  4. find_if
  5. find_if_value
  6. find_if_ptr
  7. accumulate
  8. join_with

The advantage for the `find_if` actions over the algorithms is the composability automatics of predicate functions. However, the last function given must give out a `bool`

```cpp
std::vector<std::string> strings = {"My", "Name", "Is", "Antoine"};
auto string = strings | actions::join_with(' '); // string = "My Name Is Antoine"

struct Person {
    std::string name;
}
std::vector<Person> persons;
// antoine is an std::optional<Person>
auto antoine = persons | actions::find_if(&Person::name, equal_to("Antoine"));

std::vector ints = {0, 1, 2, 3, 4, 5}
auto result = ints | actions::accumulate(0); // 0 + 0 + 1 + 2 + 3 + 4 + 5
```


You can create stateless lambda in a simple way with macro `_`.

```cpp
_((variables), expr)
auto sum = _((x, y), x + y);
auto isOdd = _((x), x % 2 == 1);
auto square = _((x), x * x);
```

## List Monad
We saw how to pipeline several algorithms. We also saw that array are _mappable_ (i.e, they are functor).
Now, imagine you have a function that takes one integer `n` and returns a list containing `n` times `n`.

```cpp
f(0) = []
f(4) = [4, 4, 4, 4]
f(5) = [5, 5, 5, 5, 5];
```  
To generate this function, you can use `make_repeater_range(value, number)` from **LTL**.
Now, I want to create an array that contains:
```cpp
[1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6...]
```

First try :
```cpp
auto f = [](int n) {return make_repeater_range(n, n);};
auto result = valueRange(0) | map(f);
```

We have the idea, but we get 

```cpp
[[1], [2, 2], [3, 3, 3], [4, 4, 4, 4], [5, 5, 5, 5, 5], [6, 6, 6, 6, 6, 6]...]
```

Here is the beauty of monads, we can join the array :

```cpp
auto f = [](int n) {return make_repeater_range(n, n);};
auto result = valueRange(0) | map(f) | join;
```

For expression like `map(f) | join`, you can use the `>>` operator

```cpp
auto f = [](int n) {return make_repeater_range(n, n);};
auto result = valueRange(0) >> map(f);
```

## Option Monad
The option monad in C++ is represented by `std::optional`
The mapping operation is as follow :

```cpp
map(optional, f) {
    if(optional == nullopt)
        return nullopt;
    else
        return make_optional(f(*optional)); 
}
```
(This code does not compile, it is just a kind of pseudo code)

Thanks to the `|` operator seen in previous chapter, you can write :

```cpp
auto square = [](auto x) {return x * x;};
std::optional<int> value = 5;
std::optional<int> empty;
auto valueSquared = value | map(square);
auto empty2 = empty | map(square);
```

What happen if `f` returns an `optional` too ? You will get a `std::optional<std::optional<T>>`.
To avoid that, you must use the `>>` operator

```cpp
struct Person {
    std::optional<std::string> address;
};

std::optional<Person> person;

std::optional<std::string> address = person >> map(&Person::address);
```

## Error Monad
The error Monad is represented in **LTL** by the type `expected<result_type, error_type>`.

The map operation is represented by

```cpp
map(expected, f) {
    if(expected.is_result()) {
        return make_expected(f(expected.result()));
    } else {
        return make_expected(expected.error());
    }
}
```

Since the `expected` type is not standard, here is a brief overview : 

```cpp
struct  value_tag  {};
struct  error_tag  {};

template  <typename  Result,  typename  Err>
class  [[nodiscard]]  expected  {
  public:
  using  value_type  =  Result;
  using  error_type  =  Err;

  static_assert(!std::is_reference_v<value_type>,  "value_type  must  not  be  a  reference");
  static_assert(!std::is_reference_v<error_type>,  "error_type  must  not  be  a  reference");

  template <typename T>
  constexpr expected(T &&t);
  
  template <typename T>
  constexpr expected(value_tag, T &&t);

  template <typename T>
  constexpr expected(error_tag,  T &&t);

  template  <typename  R,  typename  E>
  constexpr expected &operator=(expected<R, E> t);

  template  <typename  T>
  constexpr  expected  &operator=(T)  =  delete;

  constexpr  operator  bool()  const  noexcept;

  constexpr  value_type  result();
  constexpr  error_type  error();
  constexpr  bool  is_error()  const;
  constexpr  bool  is_result()  const;
};
```

As for optional, you can use `| map(f)` to process a result, and `>> map(f)` when `f` returns an expected.

## Currying
Currying is used to transform a n-arry function into a "series" of unary function `f(x, y, z)=f(x)(y)(z)`.
You can use `ltl::curry`to have such behaviour, it is inside the `ltl/functional.h` header file:

```cpp
auto f = _((x, y, z), x + y + z);
auto curriedF = ltl::curry(f);
static_assert(f(3, 4, 5) == curriedF(3, 4)(5));
```

