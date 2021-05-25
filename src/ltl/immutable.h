#pragma once

#include "ltl.h"
#include "crtp.h"
#include "Range/AsPointer.h"

namespace ltl {

template <typename T, typename Friend = void>
struct immutable_t : crtp::Comparable<immutable_t<T>> {
    friend Friend;
    immutable_t(immutable_t &&) = default;
    immutable_t(const immutable_t &) = default;

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
    immutable_t &operator=(immutable_t &&) = default;
    immutable_t &operator=(const immutable_t &) = default;

    template <typename _T>
    immutable_t &operator=(_T &&x) {
        m_object = FWD(x);
        return *this;
    }

    T &mut_ref() & { return m_object; }

  private:
    T m_object;
};

} // namespace ltl
