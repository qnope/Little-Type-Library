# Functional library

## Range like operators
You must include `ltl/operator.h` header file.
It provides several helpers to manipulate containers:

You can use the python like `enumerate` function:

```cpp
std::vector ints = {0, 1, 2, 3, 4};
for(auto [index, value]: ltl::enumerate(ints)) {
  // use index, use value
}
```

You can create stateless lambda in a simple way with macro `_`.

```cpp
_((variables), expr)
auto sum = _((x, y), x + y);
auto isOdd = _((x), x % 2 == 1);
auto square = _((x), x * x);
```

You can chain operations.

```cpp
auto vector = getVector();
auto sumOfSquaredOddNumber = accumulate(vector | ltl::filter(isOdd) | ltl::map(square), 0); 
```

**Note** : All algorithms from the std are imported into LTL. They don't need begin end, but just a container.
They also work with member function without using `std::memfn`.  

You can use also `take_n`, `take_while`, `drop_n` or `drop_while` function.

```cpp
std::vector<int> firsts = getVector() | ltl::take_n(5);
```
You will have the 5 first values of getVector()

There is also some views to iterate over a map, or vector of pointers in `ltl/Range/DefaultView.h` header file.
  * `dereference`: dereference each element of the array
  * `remove_null`: filter all elements which `static_cast<bool>(x) == true`
  * `keys` : iterate over the key of the map
  * `values`: iterate over the values of the map
  * `get(number_t<N>)`: iterate over the Nth value of the tuple inside the range. `get(0_n)` is equivalent to `keys()`

## Monads
Imagine the function you give to the map function returns a container of `int`, you'll get a range of Container of `int`.
If you want to iterate over the `int` directly, you need to _join_ all the values, you can use `join` or better, use the `operator>>`.

```cpp
std::vector<int> allInts = getVector() >> ltl::map(functionThatReturnContainerOfInt);
std::vector<int> allInts2 = getVector() | ltl::map(functionThatReturnContainerOfInt) | ltl::join;
```

The same idea apply with optional. If the function returns an optional, use `operator>>`, else use `operator|`.
```cpp
std::optional<int> opt;
auto result = opt | ltl::map(square);
// result is an optional of int, that contains either square(*opt) or nullopt;
```

## Currying
Currying is used to transform a n-arry function into a "series" of unary function `f(x, y, z)=f(x)(y)(z)`.
You can use `ltl::curry`to have such behaviour, it is inside the `ltl/functional.h` header file:

```cpp
auto f = _((x, y, z), x + y + z);
auto curriedF = ltl::curry(f);
static_assert(f(3, 4, 5) == curriedF(3, 4)(5));
```

## Other helpers:
  * `or_`: Apply `operator||` between each function given
  * `and_`: Apply `operator&&` between each function given
  * `not_`: Apply `operator!` to the function given
  * `compose`: `compose(f, g, h)(x) == h(g(f(x)))`
  * `fix`: y-combinator to make recursive lambdas.
