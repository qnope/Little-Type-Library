/**
 * @file immutable.h
 */
#pragma once

#include "ltl.h"
#include "crtp.h"
#include "Range/AsPointer.h"

namespace ltl {

/**
 *\defgroup Utils Utilitary group
 *@{
 */

template <typename T, typename Friend = void>
/**
 * @brief The immutable_t struct
 *
 * This structure is used when you want your structure like class have some const attributes.
 * Indeed, let's say you want to have something near like
 *
 * @code
 *  struct Person {
 *      const std::string name;
 *  };
 * @endcode
 *
 * The name attribute is clearly const, however, if you want to move a Person, you will have a performance impact since
 * the name will not be moved but copied
 *
 * If you declared the name without the const qualifier, the name will be mutable, and it is not a good thing too.
 *
 * The solution is to do
 *
 * @code
 *  struct Person {
 *      ltl::immutable_t<std::string> name;
 *  };
 * @endcode
 *
 * Now, name is movable, but immutable for other cases. You will not be able to modify it, just move it.
 *
 * If you want to have the possibility to modify it from your class, you may use the template argument Friend and the
 * mut ref function
 *
 * @code
 *  struct Person {
 *      void make_name_bold() {
 *          name.mut_ref() = "**" + *name + "**";
 *          // or
 *          name = "**" + *name + "**";
 *      }
 *      ltl::immutable_t<std::string, Person> name;
 *  };
 * @endcode
 */
struct immutable_t : crtp::Comparable<immutable_t<T>> {
    /// \cond
    friend Friend;
    immutable_t(immutable_t &&) = default;
    immutable_t(const immutable_t &) = default;
    immutable_t &operator=(immutable_t &&) = default;

    template <typename _T>
    immutable_t(_T &&t) noexcept : m_object{FWD(t)} {}

    operator const T &() const &noexcept { return m_object; }
    const T &operator*() const &noexcept { return m_object; }
    AsPointer<const T &> operator->() const &noexcept { return {m_object}; }

    operator T() && noexcept { return std::move(m_object); }
    T operator*() && noexcept { return std::move(m_object); }
    AsPointer<T> operator->() && noexcept { return {std::move(m_object)}; }

    T *operator&() &&noexcept = delete;
    const T *operator&() const &noexcept { return std::addressof(m_object); }

    friend auto operator==(const immutable_t &a, const immutable_t &b) noexcept { return *a == *b; }

    friend auto operator<(const immutable_t &a, const immutable_t &b) noexcept { return *a < *b; }

  private:
    immutable_t &operator=(const immutable_t &) = default;

    template <typename _T>
    immutable_t &operator=(_T &&x) {
        m_object = FWD(x);
        return *this;
    }

    /// \endcond

    T &mut_ref() & { return m_object; }

  private:
    T m_object;
};

/// @}

} // namespace ltl
