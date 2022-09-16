# Type library
Here we are going to discuss about the type library and the header `ltl/ltl.h`.

## overloader
The `overloader` is a simple tools that allows you to make lambda overloadable. Example:

```cpp
auto intLambda = [](int x){std::cout << x << "\n";};
auto doubleLambda = [](double x){std::cout << x << "\n";};
auto overloadedLambda = ltl::overloader{intLambda, doubleLambda};
overloadedLambda(5); // intLambda
overloadedLambda(5.0); // doubleLambda
```
## bool_t and number_t
Those two types have the same kind of purpose. It allows to define directly a value as a type. `false_t` and `true_t` are not of the same type, but they are both a kind of `bool_t<>`.
The same things is used on `number_t`. `number_t<0>` is not of the same type `number_t<1>`.

They act like `std::integral_constant` or `std::bool_constant` in the C++ standard library. However, they are easier to use.
LTL provides helper variables: `false_v` and `true_v` for boolean constant, and `ltl::number_v<int>` for integral constant.
Actually, there is even a better way to create compile time number : `operator_n`.
As such, write `0_n` is strictly equivalent to `ltl::number_v<0>`.

You can use the normal operators on `number_t`. `static_assert(5_n * 3_n == 18_n + 2_n - 5_n);`

## A variable that carry a type.
Thanks to the `ltl::type_t` type, you can carry a type inside a variable. Since you can manipulate type like a normal variable,
meta programming will be easier. LTL provides `ltl::type_v<typename>` constexpr variable. `type_t`is comparable:
`static_assert(ltl::type_v<int> != ltl::type_v<double>)`

## Some macros

  * `decltype_t(t)`: extract the type from a `type_t`
  * `type_from(expr)`: transform `expr` by `ltl::type_v<decltype(x)>`. `static_assert(type_from(5) == ltl::type_v<int>);`
  * `decay_from(expr)`: the same of `type_from`, but after decaying
  * `if_constexpr(c)`: equivalent to `if constexpr` but work directly with the type. Can help because some compilers are bugged when dealing with constant expression on non constant objects
  * `else_if_constexpr(c)`
  * `compile_time_error(msg, T)`: Can be used to generate compile time error.
  * `FWD(x)`: For perfect forwarding : does not work for lambda capture !
  * `lift(f)`: Wraps a function within a generic lambda.
  * `typed_static_assert(c)` : equivalent to `static_assert` but works directly with the type
  * `typed_static_assert_msg(c, msg)`
  
## CRTP

Let's say you have an object that can be `Comparable`. You are going to write all the following operators : `==,!=,<,<=,>,>=`. However, some are deducible from other. Using `ltl::Comparable` base class will let you to just write `==`and `<` and other will be automatically deduced.

```cpp
class MyObject : public ltl::Comparable<MyObject> {
public:
    friend auto operator<(const MyObject&);
    friend auto operator==(const MyObject&);
private:
};
``` 

If you just provide the `==` operator, only the `!=` operator will be deduced.

You have the same behaviour for :

  * PostIncremental
  * PostDecremental
  * Additionnable
  * Subtractable
  
When you want to make a crtp base class, you can use `ENABLE_CRTP` macro.
