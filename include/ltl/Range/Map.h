#pragma once

#include "BaseIterator.h"

namespace ltl {
template <typename It, typename Function>
struct MapIterator : BaseIterator<MapIterator<It, Function>, It, Function> {
    using reference = std::invoke_result_t<Function, typename std::iterator_traits<It>::reference>;
    DECLARE_EVERYTHING_BUT_REFERENCE

    using BaseIterator<MapIterator<It, Function>, It, Function>::BaseIterator;

    reference operator*() const { return this->m_function(*this->m_it); }
    pointer operator->() const { return this->m_function(*this->m_it); }
};

template <typename F>
struct MapType {
    F f;
};
template <typename F>
constexpr auto map(F &&f) {
    return MapType<std::decay_t<F>>{FWD(f)};
}

LTL_MAKE_IS_KIND(MapIterator, is_map_iterator, IsMapIterator, typename);
LTL_MAKE_IS_KIND(MapType, is_map_type, IsMapType, typename);

} // namespace ltl
