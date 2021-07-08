<img src="LTL.png" alt="drawing" width="1000"/>
## Introduction
**Little Type Library** or **LTL** for short is a lightweight library that can be used for meta programming and / or functional programming.
**LTL** is under MIT license, and as such, you can use it without any limitation. I just ask you to signal me every bugs you are going to experience.
This library works under C++ 17.

There is some features that are heavily inspired from C++20 range proposal.

## The different sub-library
+ [Type library](documentation/Type.md):  
  1. Variable carrying type
  2. Compile time computation
  3. Compile time programming in the same way of run time programming
  4. CRTP
+ [Traits library](documentation/Traits.md)
  1. Traits like ordinary function
  2. Concept like
  3. Macro to generate your own traits for your templated types
+ [Tuple library](documentation/Tuple.md)
  1. Tuple like array with square brackets notation
  2. Type manipulation with type list
  3. For each for tuples
+ [Algorithms library](documentation/algorithms.md)
  1. Predicates
  2. No need for iterators
  3. Member function are allowed in algorithms
  4. Find returns option monad
  5. zip
  6. index and enumerate
+ [Functional library](documentation/Functional.md)
  1. Pipeline
  2. Infinite sequence
  3. Actions
  4. List monad
  5. Option monad
  6. Error monad
  7. Currying
+ [Other utilities](documentation/Other.md)
  1. Condition helpers (`AllOf`, `AnyOf`, `NoneOf`)
  2. Streambuf for write into an array thanks to standard streams
  3. Strong types
  4. Variant helpers and `recursive_variant`

I would like to thanks Gaétan Bahl and Nicolas Tollenaere for their advice
I would also thank [friendlyanon](https://github.com/friendlyanon) for the better CMake usage
