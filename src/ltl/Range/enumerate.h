/**
 * @file enumerate.h
 */
#pragma once

#include "Value.h"
#include "Zip.h"

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

template <typename Container>
/**
 * @brief enumerate - enumerate elements with a tuple of index, element
 *
 * @code
 *  std::vector<std::string> strings;
 *
 *  for(const auto &[index, string]: ltl::enumerate(strings)) {
 *      use(string);
 *  }
 *
 * @endcode
 * @param container
 */
auto enumerate(Container &&container) {
    using std::size;
    return zip(valueRange<std::size_t>(0), FWD(container));
}

/// @}
} // namespace ltl
