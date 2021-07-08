/**
 * @file Map.h
 */
#pragma once

#include "ltl/concept.h"
#include "ltl/functional.h"
#include "ltl/optional_type.h"

#include "Join.h"
#include "Range.h"

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

/// \cond
template <typename It, typename Function>
struct MapIterator :
    BaseIterator<MapIterator<It, Function>, It>,
    WithFunction<Function>,
    IteratorOperationWithDistance<MapIterator<It, Function>>,
    IteratorSimpleComparator<MapIterator<It, Function>> {
    using reference = fast::remove_rvalue_reference_t<decltype(fast_invoke(
        std::declval<Function>(), std::declval<typename std::iterator_traits<It>::reference>()))>;

    DECLARE_EVERYTHING_BUT_REFERENCE(get_iterator_category<It>);

    MapIterator() = default;

    MapIterator(It current, Function f) noexcept :
        BaseIterator<MapIterator, It>{std::move(current)}, //
        WithFunction<Function>{std::move(f)} {}

    reference operator*() const { return this->m_function(*this->m_it); }
};

template <typename F>
struct MapType {
    F f;
};

/// \endcond

template <typename... Fs>
/**
 * @brief map - Used to perform map operation on a functor
 *
 * Apply the composition of fs to the functor. The functor can be different things.
 * * It may be an array
 * * It may be an optional
 * * It may be an expected
 * We use it through the `operator |`
 *
 * @code
 *  std::vector<int> numbers;
 *
 *  // Here we get the square of the numbers as string
 *  std::vector<std::string> strings_square = numbers | ltl::map(square, lift(std::to_string));
 * @endcode
 *
 * We may apply the operation with a monadic behaviour using `operator>>`
 *
 * @code
 *  std::vector<int> f(int value);
 *  std::vector<int> numbers;
 *
 *  std::vector<std::vector<int>> result = numbers | ltl::map(f);
 *  std::vector<int> result = numbers | ltl::map(f) | ltl::join;
 *  std::vector<int> result = numbers >> ltl::map(f);
 * @endcode
 *
 * For an array, the `operator>>` is equivalent to `operator|` and a `join`
 *
 * @param fs
 */
constexpr auto map(Fs... fs) {
    auto foo = compose(std::move(fs)...);
    return MapType<decltype(foo)>{std::move(foo)};
}

template <typename F, typename... Fs, requires_f(!IsIterable<F>)>
/**
 * @brief transform - Same as ltl::map(f, fs...)
 * @param f
 * @param fs
 */
constexpr auto transform(F f, Fs... fs) {
    return map(std::move(f), std::move(fs)...);
}

/// \cond

template <typename F>
struct is_chainable_operation<MapType<F>> : true_t {};

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    using std::begin;
    using std::end;
    using it = decltype(begin(FWD(a)));
    return Range{MapIterator<it, decltype(b.f)>{begin(FWD(a)), b.f}, //
                 MapIterator<it, decltype(b.f)>{end(FWD(a)), b.f}};
}

template <typename T1, typename F, requires_f(IsOptional<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    if (a)
        return std::make_optional(ltl::fast_invoke(std::move(b.f), *FWD(a)));
    return decltype(std::make_optional(ltl::fast_invoke(std::move(b.f), *FWD(a)))){};
}

template <typename T1, typename F, requires_f(IsOptionalType<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    if_constexpr(a.has_value) { //
        return ltl::optional_type{ltl::fast_invoke(std::move(b.f), *a)};
    }
    else return ltl::nullopt_type;
}

template <typename T1, typename F, requires_f(IsIterable<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapType<F> b) {
    return FWD(a) | std::move(b) | join;
}

template <typename T1, typename F, requires_f(IsOptional<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapType<F> b) {
    static_assert(IsOptional<decltype(ltl::fast_invoke(std::move(b.f), *a))>,
                  "With >> notation, function must return optional");
    if (a)
        return ltl::fast_invoke(std::move(b.f), *FWD(a));
    return decltype(ltl::fast_invoke(std::move(b.f), *FWD(a))){};
}

template <typename T1, typename F, requires_f(IsOptionalType<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapType<F> b) {
    if_constexpr(a.has_value) { //
        static_assert(IsOptionalType<decltype(ltl::fast_invoke(std::move(b.f), *a))>,
                      "With >> notation, function must return optional_type");
        return ltl::fast_invoke(std::move(b.f), *a);
    }
    else {
        return ltl::nullopt_type;
    }
}

template <typename It, typename Function>
struct MapCachedIterator :
    BaseIterator<MapCachedIterator<It, Function>, It>,
    WithFunction<Function>,
    WithSentinel<It>,
    IteratorSimpleComparator<MapCachedIterator<It, Function>> {
    using result_type = fast::remove_rvalue_reference_t<decltype(fast_invoke(
        std::declval<Function>(), std::declval<typename std::iterator_traits<It>::reference>()))>;

    using reference = typename ltl::remove_cvref_t<decltype(*std::declval<result_type>())>::underlying_type;
    DECLARE_EVERYTHING_BUT_REFERENCE(std::input_iterator_tag);

    MapCachedIterator() = default;

    MapCachedIterator(It current, It end, Function f) noexcept :
        BaseIterator<MapCachedIterator, It>{std::move(current)}, //
        WithFunction<Function>{std::move(f)}, WithSentinel<It>{{}, std::move(end)} {
        if (this->m_it != this->m_sentinelEnd) {
            m_result = this->m_function(*this->m_it);
        }
    }

    reference operator*() const { return **const_cast<MapCachedIterator &>(*this).m_result; }

    MapCachedIterator &operator++() noexcept {
        ++this->m_it;
        if (this->m_it != this->m_sentinelEnd) {
            m_result = this->m_function(*this->m_it);
        }
        if (!m_result)
            this->m_it = this->m_sentinelEnd;
        return *this;
    }

    MapCachedIterator &operator--() = delete;

  private:
    result_type m_result;
};

template <typename F>
struct MapCachedType {
    F f;
};

template <typename... Fs>
constexpr auto map_cached(Fs... fs) {
    auto foo = compose(std::move(fs)...);
    return MapCachedType<decltype(foo)>{std::move(foo)};
}

template <typename F, typename... Fs, requires_f(!IsIterable<F>)>
constexpr auto transform_cached(F f, Fs... fs) {
    return map_cached(std::move(f), std::move(fs)...);
}

template <typename F>
struct is_chainable_operation<MapCachedType<F>> : true_t {};

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapCachedType<F> b) {
    using std::begin;
    using std::end;
    using it = decltype(begin(FWD(a)));
    return Range{MapCachedIterator<it, decltype(b.f)>{begin(FWD(a)), end(FWD(a)), b.f}, //
                 MapCachedIterator<it, decltype(b.f)>{end(FWD(a)), end(FWD(a)), b.f}};
}

template <typename T1, typename F, requires_f(IsIterable<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapCachedType<F> b) {
    return FWD(a) | std::move(b) | join;
}

/// \endcond

template <typename T>
/**
 * @brief make_move_range - Will move each elements of the array
 * @param t
 */
auto make_move_range(T &&t) {
    auto move = [](auto &x) -> ltl::remove_cvref_t<decltype(x)> { //
        return std::move(x);
    };
    return FWD(t) | ltl::map(move);
}

/// @}

} // namespace ltl
