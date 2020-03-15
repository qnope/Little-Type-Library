#pragma once

#include "Tuple.h"

namespace ltl {
//////////////////////////////// Y_Combinator /////////////////////////////////
template <typename F> struct fix : F {
  template <typename... Args>
  constexpr auto operator()(Args &&... args) const
      -> decltype(std::declval<const F &>()(std::declval<const fix &>(),
                                            std::declval<Args>()...)) {
    return static_cast<const F &>(*this)(*this, FWD(args)...);
  }

  template <typename... Args>
  constexpr auto operator()(Args &&... args)
      -> decltype(std::declval<F &>()(std::declval<fix &>(),
                                      std::declval<Args>()...)) {
    return static_cast<F &>(*this)(*this, FWD(args)...);
  }
};
template <typename F> fix(F)->fix<F>;

template <typename F, typename... Args>
constexpr auto report_call(F f, Args... xs) {
  return [f = std::move(f),
          xs = tuple_t{std::move(xs)...}](auto &&... _ys) -> decltype(auto) {
    auto ys = tuple_t<decltype(_ys)...>(FWD(_ys)...);
    auto args = xs + std::move(ys);
    return args([f](auto &&... args) -> decltype(auto) {
      return ltl::invoke(f, FWD(args)...);
    });
  };
}

template <typename F, typename... Args>
constexpr decltype(auto) curry(F f, Args &&... args) {
  if_constexpr(ltl::is_invocable(f, FWD(args)...)) {
    return ltl::invoke(f, FWD(args)...);
  }
  else {
    return report_call(lift(curry), std::move(f), FWD(args)...);
  }
}

namespace detail {
template <typename... Fs> constexpr auto composeImpl(ltl::tuple_t<Fs...> fs) {
  auto f = std::move(fs[fs.length - 1_n]);
  auto tails = std::move(fs).pop_back();
  if_constexpr(tails.isEmpty) { return f; }
  else {
    return [f = std::move(f),
            tails = std::move(tails)](auto &&... xs) -> decltype(auto) {
      return ltl::invoke(f, composeImpl(tails)(FWD(xs)...));
    };
  }
}

} // namespace detail
template <typename... Fs> constexpr auto compose(Fs... fs) {
  return detail::composeImpl(ltl::tuple_t<Fs...>{std::move(fs)...});
}

template <typename F> constexpr auto not_(F f) {
  return
      [f = std::move(f)](auto &&... xs) { return !ltl::invoke(f, FWD(xs)...); };
}

template <typename... Fs> constexpr auto or_(Fs... fs) {
  return
      [fs...](auto &&... xs) { return (false_v || ... || (fs(FWD(xs)...))); };
}

template <typename... Fs> constexpr auto and_(Fs... fs) {
  return [fs...](auto &&... xs) { return (true_v && ... && (fs(FWD(xs)...))); };
}

} // namespace ltl
