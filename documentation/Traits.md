# Traits library
## Trait as a normal function
Most of the standard traits are available in LTL.
They are in the header `ltl/traits.h`

```cpp
auto ptrInt = ltl::type_v<int*>;
auto typeChar = ltl::type_v<char>;
typed_static_assert(ltl::is_pointer(ptrInt));
typed_static_assert(ltl::is_pointer(typeChar));
```

True types are also available in this process:

```cpp
int *p = nullptr;
typed_static_assert(ltl::is_pointer(p));
```

## Concept like macro
Include the header `ltl/concept.h`.
You can write: 

```cpp
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
```

For custom templated types, you can use `LTL_MAKE_IS_KIND` macro to generate appropriate traits and concept.

```cpp
LTL_MAKE_IS_KIND(type, name, conceptName, kind of template);
LTL_MAKE_IS_KIND(std::optional, is_optional, IsOptional, typename);
```
