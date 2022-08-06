/**
 * @file DefaultView.h
 */
#pragma once

#include "ltl/Tuple.h"
#include "Filter.h"
#include "Map.h"

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

/**
 * @brief dereference - returns a transform function that dereferrence objects
 *
 * @code
 *  std::vector<std::unique_ptr<int>> ptrs;
 *  auto values = ptrs | ltl::map(ltl::dereference());
 *  auto values2 = ptrs | ltl::dereference()
 * @endcode
 */
inline auto dereference() noexcept {
    return map([](auto &&x) noexcept -> fast::remove_rvalue_reference_t<decltype(*FWD(x))> { return *FWD(x); });
}

/**
 * @brief remove_null - returns a filter that remove null objects
 *
 * @code
 *  std::vector<std::optional<int>> objects;
 *  auto not_null_objects = objects | ltl::remove_null();
 * @endcode
 */
inline auto remove_null() noexcept {
    return filter([](auto &&x) noexcept { return static_cast<bool>(FWD(x)); });
}

/**
 * @brief to_ptr - A transform function that returns the address of an object
 *
 * @code
 *  std::vector<int> values;
 *  std::vector<int*> ptrs = values | ltl::map(ltl::to_ptr);
 * @endcode
 */
inline auto to_ptr = [](auto &x) noexcept { return std::addressof(x); };

/**
 * @brief to_bool - A transform function that casts an object to bool
 */
inline auto to_bool = [](auto &&x) noexcept { return static_cast<bool>(x); };

/**
 * @brief to_pair - A transform function that transform a tuple of 2 elements into a pair
 *
 * @code
 *  std::vector<int> xs;
 *  std::vector<std::string> ys;
 *
 *  std::map<int, std::string> zs = ltl::zip(xs, ys) | ltl::map(ltl::to_pair);
 * @endcode
 */
inline auto to_pair = [](auto &&tuple) {
    using Tuple = ltl::remove_cvref_t<decltype(tuple)>;
    static_assert(std::tuple_size_v<Tuple> == 2, "Tuple must have two elements");
    using pair = std::pair<std::tuple_element_t<0, Tuple>, std::tuple_element_t<1, Tuple>>;
    return pair{FWD(tuple)[0_n], FWD(tuple)[1_n]};
};

template <int N>
/**
 * @brief get - Generalization of ltl::keys and ltl::values for nth values
 */
auto get(number_t<N>) {
    return map([](auto &&x) noexcept -> decltype(auto) { return (::std::get<N>(FWD(x))); });
}

/**
 * @brief keys - may be used to iterate on the keys of one map
 *
 * @code
 *  std::map<std::string, int> map;
 *  for(const std::string &key : map | ltl::keys()) {
 *
 *  }
 * @endcode
 */
inline auto keys() { return get(0_n); }

/**
 * @brief keys - may be used to iterate on the values of one map
 *
 * @code
 *  std::map<std::string, int> map;
 *  for(int &key : map | ltl::values()) {
 *
 *  }
 * @endcode
 */
inline auto values() { return get(1_n); }

/// @}

} // namespace ltl
