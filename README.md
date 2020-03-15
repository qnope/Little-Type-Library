# Little Type Library (LTL)

## Introduction
**Little Type Library** or **LTL** for short is a lightweight library that can be used for meta programming and / or functional programming. It can be used instead of **boost::hana** or equivalent because they are too heavy or for whatever reason.
**LTL** is under MIT license, and as such, you can use it without any limitation. I just ask you to signal me every bugs you are going to experience.
The library is tested on :
1. GCC 7.1 to the latest version
2. Clang 7.0 to the latest version
3. MSVC latest version

This library works under C++ 17.
## The different sub-library
+ Type library:  
  1. Variable carrying type
  2. Compile time computation
  3. Compile time programming in the same way of run time programming
+ Traits library
  1. Traits like ordinary function
  2. Concept like
  3. Macro to generate your own traits for your templated types
+ Tuple library
  1. Tuple like array with square brackets notation
  2. Type manipulation with type list
  3. For each for tuples
+ Functional library
  1. Range like operators
  2. Range like algorithms that works with member functions
  3. **List** and **Maybe** monads
+ Other utilities
  1. Condition helpers (`AllOf`, `AnyOf`, `NoneOf`)
  2. Streambuf for write into an array thanks to standard streams
  3. Strong types
  4. Variant helpers and `recursive_variant`