/**
 * @file VariantUtils.h
 */
#pragma once

#include "functional.h"
#include "traits.h"
#include "fast.h"
#include <memory>
#include <variant>

namespace ltl {

/**
 *\defgroup Utils Utilitary group
 *@{
 */

template <typename V, typename... Fs>
/**
 * @brief match Simple variant visitation
 *
 * It is roughly equivalent to `std::visit(overloader{FWD(fs)...}, FWD(v));`
 */
constexpr decltype(auto) match(V &&v, Fs &&... fs) {
    return ::std::visit(overloader{FWD(fs)...}, FWD(v));
}

template <typename Variant, typename... Fs>
/**
 * @brief match_result This is the same function as match, but it returns a variant of results
 */
constexpr auto match_result(Variant &&variant, Fs... fs) {
    using qualified_types = typename fast::qualified_type_list<Variant>::type;
    using result_from_function = fast::function_to_metafunction<overloader<Fs...>>;
    using result_types = typename fast::apply<qualified_types, result_from_function::template apply>::type;
    using result_type = typename fast::rename<result_types, std::variant>::type;
    auto function = overloader{std::move(fs)...};
    return std::visit([&function](auto &&x) -> result_type { return function(FWD(x)); }, FWD(variant));
}

template <typename F, typename Variant>
/**
 * @brief is_callable_from - To know if the function F can handle all the types of the variant
 */
constexpr auto is_callable_from(F &&, Variant &&) {
    using qualified_types = typename fast::qualified_type_list<Variant>::type;
    return bool_v<fast::all_of_v<qualified_types, fast::bind1st<std::is_invocable, F>::template apply>>;
}

/// \cond

template <typename T>
class recursive_wrapper {
  public:
    static constexpr auto type = type_v<T>;

  public:
    recursive_wrapper(T &&t) noexcept : m_ptr{std::make_unique<T>(std::move(t))} {}

    recursive_wrapper(const recursive_wrapper &other) = delete;
    recursive_wrapper(recursive_wrapper &&other) noexcept = default;

    recursive_wrapper &operator=(const recursive_wrapper &) = delete;
    recursive_wrapper &operator=(recursive_wrapper &&other) noexcept = default;

    recursive_wrapper &operator=(T &&v) {
        m_ptr = std::make_unique<T>(std::move(v));
        return *this;
    }

    T &operator*() noexcept { return *m_ptr; }
    const T &operator*() const noexcept { return *m_ptr; }

    T *operator->() noexcept { return std::addressof(*m_ptr); }
    const T *operator->() const noexcept { return std::addressof(*m_ptr); }

  private:
    std::unique_ptr<T> m_ptr;
};

LTL_MAKE_IS_KIND(recursive_wrapper, is_recursive_wrapper, IsRecursiveWrapper, typename, );

/// \endcond

template <typename... Ts>
/**
 * @brief The recursive_variant class - A variant that supports recursive type
 */
class recursive_variant {
    /// \cond
  public:
    template <typename T>
    recursive_variant(T &&v) noexcept {
        m_variant = FWD(v);
    }

    recursive_variant(const recursive_variant &) = delete;
    recursive_variant(recursive_variant &&v) noexcept { m_variant = std::move(v.m_variant); }

    recursive_variant &operator=(const recursive_variant &) = delete;
    recursive_variant &operator=(recursive_variant &&v) noexcept {
        m_variant = std::move(v.m_variant);
        return *this;
    }

    template <typename T>
    recursive_variant &operator=(T &&v) {
        m_variant = FWD(v);
        return *this;
    }

    template <typename F, typename... Variants>
    friend decltype(auto) recursive_visit(F &&f, Variants &&... variants) {
        std::visit(
            [&f](auto &&... xs) {
                auto unwrap = [](auto &x) -> decltype(auto) {
                    if constexpr (IsRecursiveWrapper<decltype(x)>) { //
                        return *x;
                    } else {
                        return (x);
                    }
                };
                ltl::invoke(static_cast<F &&>(f), unwrap(FWD(xs))...);
            },
            FWD(variants).m_variant...);
    }

    /// \endcond

  private:
    std::variant<Ts...> m_variant;
};

/// @}

} // namespace ltl
