# Tuple library
Tuple are now really well implemented and used word wide.
However, writing `std::get<0>(myTuple)` is annoying.

## Tuple with squared brackets
To enable this feature, you must include `ltl/Tuple.h` header file.
The tuple type is `ltl::tuple_t`

```cpp
auto tuple = ltl::tuple_t<int, double, std::string>{};
tuple[0_n] = 8;
tuple[1_n] = 8.0;
tuple[2_n] = "LTL is great"s;
```

They are some helpers type:

```cpp
/// Convenience types
template <typename... Types> using type_list_t = tuple_t<type_t<Types>...>;
template <int... Ns> using number_list_t = tuple_t<number_t<Ns>...>;
template <bool... Bs> using bool_list_t = tuple_t<bool_t<Bs>...>;

// Convenience variables
template <typename... Types> constexpr type_list_t<Types...> type_list_v{};
template <int... Ns> constexpr number_list_t<Ns...> number_list_v{};
template <bool... Bs> constexpr bool_list_t<Bs...> bool_list_v{};
```

## Tuple algorithms
Algorithms for tuple are in the `ltl/tuple_algos.h` header file.

Firstly, tuple are _foreach able_ and _apply able_.

```cpp
auto tuple = ltl::tuple_t{10, 18.0}; // CTAD tuple_t<int, double>
auto sum = tuple([](auto ...xs) {return 0 + ... + xs;}; // double(28.0)
for_each(tuple, [](auto x){std::cout << x << "\n";});
```

They are transformable

```cpp
auto tuple = ltl::tuple_t{10, 18.0};
auto tuple_string = ltl::transform_type(tuple, lift(std::to_string)); // tuple<string, string>("10", "18");
```

When you are dealing with a `type_list_t`, you can use following algorithms:
  * `find_type(tuple, type)`: Returns an `optional_type<IndexType>`
  * `find_if_type(tuple, predicate)`: Returns an `optional_type<IndexType>`
  * `contains_type(tuple, type)`: Returns `true_v` or `false_v` if type is contained in tuple
  * `contains_if_type(tuple, predicate)` : Returns `true_v` or `false_v` if one type of tuple satisfied the predicate 
  * `count_type(tuple, type)`: Returns the number of occurence of type in tuple
  * `count_if_type(tuple, predicate)`: Returns the number of occurence of types that satisfy the predicate
  * `all_of_type(tuple, predicate)`: Returns `true_v` if all types satisfy the predicate
  * `any_of_type(tuple, predicate)` : Returns `true_v` if at least one type satisfy the predicat e
  * `none_of_type(tuple, predicate)` : Returns `!any_of_type(tuple, predicat)`
  * `unique_type(tuple)`: Returns a `type_list_t` without duplicates
  * `filter_type(tuple, predicate)` : Filter the list and remove types that does not satisfy the predicate
