
# Algorithms library

Here we are going to discuss about standard algorithms and what **LTL** can bring to them.

## Predicates
Let's say we have a list of numbers and we want to check if there is one that is less than 8
Thanks `std::any_of`algorithm, we can write something close to :
```cpp
std::array values = {0, 1, 2, 6, 9, 8, 14, 58, 65};
bool result = std::any_of(begin(values), end(values), [](int x) {return x < 8;});
```

Now, say we want to know if there is a number that is less than 8 or greater than 18

```cpp
bool result = std::any_of(begin(values), end(values), [](int x) {
	return (x < 8) || (x > 18);
});
```
It is all easy  but there is a lot of boiler plate...

**LTL** provides some predicates and some helper function to deal with such problems:
   * `less_than(x)`
   * `less_than_equal(x)`
   * `equal_to(x)`
   * `greater_than(x)`
   * `greater_than_equal(x)`
   * `not_equal_to(x)`
   * `and_(predicates...)`
   * `or_(predicates...)`
   * `not_(f)`
   * `byAscending(f)`
   * `byDescending(f)`
   * `compose(fs)`: It is the reverse composition : `compose(f, g)(x) = g(f(x))`

Knowing that the examples above can be written as following
```cpp
bool result1 = std::any_of(begin(values), end(values), less_than(8));
bool result2 = std::any_of(begin(values), end(values), or_(less_than(8), greater_than(18)));
```

## Removing the need of iterators
**LTL** provides all algorithms without the need to use iterators. The code above could be written as:
```cpp
bool result1 = ltl::any_of(values, less_than(8));
bool result2 = ltl::any_of(values, or_(less_than(8), greater_than(18)));
```
## Using pointer to member function of pointer to member attributes
Let's say we have the **Bottle** type :
```cpp
struct Bottle {
  bool isEmpty;
};
```
We want to know if there is one empty bottle in our collections, we can use a pointer to the member attributes

```cpp
std::vector<Bottle> bottles;
bool result = ltl::any_of(bottles, &Bottle::isEmpty);
```

## Find functions
C++17 brought us `std::optional`which is an equivalent to the `Option` monad in functional languages. The standard `find`(and `find_if`...) algorithm returns an iterator. The iterator is valid if the algorithm found the value, and is equals to the end of the container if the algorithm does not.
It makes mandatory to write such things :
```cpp
std::vector<Bottle> bottles;

if(auto itEmptyBottle = ltl::find_if(bottles, &Bottle::isEmpty);
   itEmptyBottle != end(bottles)) {
    fillBottle(*itEmptyBottle);
}
```
**LTL** provides `find_value`, `find_ptr` (also for `find_if` and `find_if_not`).
The function with `_value`suffixes returns a `std::optional<T>` (here a `std::optional<Bottle>`) and the function with `_ptr`returns a pointer`T*`.

It allows us to write
```cpp
std::vector<Bottle> bottles;

if(auto emptyBottle = ltl::find_if_value(bottles, &Bottle::isEmpty)) {
    display(*emptyBottle); // It is useless to modify emptyBottle since it is a copy
}
if(auto *emptyBottle = ltl::find_if_ptr(bottles, &Bottle::isEmpty)) {
    fill(*emptyBottle);
}
```

## Zip
Sometimes, it happens that we have two correlated collections
```cpp
std::vector<int> v1;
std::vector<int> v2;

assert(v1.size() == v2.size());

for(int i = 0; i < v1.size(); ++i) {
    use(v1[i], v2[i]);
}
```

**LTL** provides a `zip`function that make things easier

```cpp
std::vector<int> v1;
std::vector<int> v2;

for(auto [a, b] : ltl::zip(v1, v2)) {
    use(a, b);
}
// or
ltl::for_each(ltl::zip(v1, v2), ltl::unzip(use));
```

`unzip` is a function that takes a function as argument and returns a callable that takes something _applyable_ like a `std::pair`,  `std::tuple` or a `ltl::tuple_t` and forward it to the function given to `unzip`

For Python lovers, we provide also `enumerate`

```cpp
for(auto [index, value] : enumerate(array)) {
    use(index, value);
}
```

## Index
It can happen that we need to find the index of an element when we are dealing with containers. In C++, we often saw such a code :
```cpp
std::vector<int> array;
auto result = -1;

for(int i = 0u; i < array.size(); ++i) {
   if(array[i] == 18) {
       result = i;
       break;
   }
}

if(result != -1)
    use(array[result]);
    
// or

auto it = ltl::find(array, 18);
auto result = std::distance(begin(array), it);

if(result != array.size())
    use(array[result]);
```
It is a lot of boilerplate. As such, **LTL** provides `index_of` and `index_if` functions that returns an `std::optional<std::size_t>`

```cpp
auto is_odd = [](auto x){return x % 2 == 1;};
std::vector array = {0, 1, 2, 3, 4, 5, 6, 7};

if(auto index = ltl::index_of(array, 6)) {
    use(*index);
}

if(auto index = ltl::index_if(array, is_odd)) {
    use(*index);
}
```