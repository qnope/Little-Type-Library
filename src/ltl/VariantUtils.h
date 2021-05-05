#pragma once

#include "functional.h"
#include "traits.h"
#include "tuple_algos.h"
#include <memory>
#include <variant>

namespace ltl {
template <typename V, typename... Fs>
decltype(auto) match(V &&v, Fs &&... fs) {
    return ::std::visit(overloader{FWD(fs)...}, FWD(v));
}

template <typename Variant, typename... Fs>
auto match_result(Variant &&variant, Fs... fs) {
    constexpr auto qualified_types = types_from(variant);
    overloader function{fs...};
    constexpr auto result_from_function = [f = type_from(function)](auto type) { return invoke_result(f, type); };
    using result_types = decltype(transform_type(qualified_types, result_from_function));

    using result_type = build_from_type_list<std::variant, result_types>;
    return std::visit([&function](auto &&x) -> result_type { return function(FWD(x)); }, FWD(variant));
}

template <typename F, typename Variant>
constexpr auto is_callable_from(F &&, Variant &&variant) {
    auto qualified_types = types_from(variant);
    return qualified_types(
        [](auto... xs) { return bool_v<std::conjunction_v<std::is_invocable<F, extract_type<decltype(xs)>>...>>; });
}

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

LTL_MAKE_IS_KIND(recursive_wrapper, is_recursive_wrapper, is_recursive_wrapper_f, IsRecursiveWrapper, typename, );

template <typename... Ts>
class recursive_variant {
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

  private:
    std::variant<Ts...> m_variant;
};

} // namespace ltl
