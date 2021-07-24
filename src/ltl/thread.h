#pragma once

#include "ltl.h"
#include <mutex>
#include <shared_mutex>

namespace ltl {
/**
 *\defgroup Threads Thread group
 *@{
 */

template <typename T>
/**
 * @brief The Mutex class
 *
 * This class is done to protect a variable via mutual exclusion
 *
 * @code
 *  int myVariable;
 *  ltl::mutex<int> variable;
 *
 *  variable.with_lock([&](auto &value) {
 *      use(value); // here value is a int&
 *  });
 *
 *  std::as_const(variable).with_lock([&](auto &value) {
 *      use(value); // here value is a const int&
 *  });
 * @endcode
 */
class mutex {
  public:
    template <typename F>
    /**
     * @brief with_lock Lock as a mutable reference
     */
    decltype(auto) with_lock(F &&f) {
        std::lock_guard lock{m_mutex};
        return ltl::invoke(FWD(f), m_value);
    }

    template <typename F>
    /**
     * @brief with_lock lock as a const reference
     */
    decltype(auto) with_lock(F &&f) const {
        std::shared_lock lock{m_mutex};
        return ltl::invoke(FWD(f), m_value);
    }

  protected:
    T m_value;
    mutable std::shared_mutex m_mutex;
};

/// @}

} // namespace ltl
