#pragma once

#include <iterator>
#include <streambuf>

#include "algos.h"
#include "traits.h"

namespace ltl {
using std::begin;
using std::end;
template <typename Container, typename Char,
          typename Trait = std::char_traits<Char>>
class basic_readonly_streambuf final
    : public std::basic_streambuf<Char, Trait> {
public:
  basic_readonly_streambuf(Container container) noexcept
      : m_container{std::move(container)} {
    computeIterators();
  }

public:
  template <typename OtherContainer>
  void feed(const OtherContainer &container) {
    using std::size;
    const auto number = std::distance(m_begin, m_current);
    m_container.reserve(m_container.size() + size(container));
    ltl::copy(container, std::back_inserter(m_container));
    computeIterators();
    std::advance(m_current, number);
  }

protected:
  std::streamsize showmanyc() override {
    return std::distance(m_current, m_end);
  }

  std::streamsize xsgetn(typename Trait::char_type *s,
                         std::streamsize count) override {
    auto realCount = std::min(showmanyc(), count);
    std::copy_n(m_current, realCount, s);
    std::advance(m_current, realCount);
    return realCount;
  }

  typename Trait::int_type uflow() override {
    if (m_current != m_end) {
      return Trait::to_int_type(*m_current++);
    } else {
      return Trait::eof();
    }
  }

  typename Trait::int_type underflow() override {
    if (m_current != m_end) {
      return Trait::to_int_type(*m_current);
    } else {
      return Trait::eof();
    }
  }

  int sync() override {
    auto is_erasable = IS_VALID((c, beg, cur), c.erase(m_begin, m_current));

    if_constexpr(is_erasable(m_container)) {
      m_container.erase(m_begin, m_current);
    }
    else {
      Container newContainer;
      newContainer.reserve(std::distance(m_current, m_end));
      std::copy(m_current, m_end, std::back_inserter(newContainer));
      m_container = std::move(newContainer);
    }
    computeIterators();
    return 0;
  }

private:
  void computeIterators() {
    m_end = end(m_container);
    m_begin = begin(m_container);
    m_current = begin(m_container);
  }

private:
  Container m_container;
  decltype(begin(m_container)) m_begin;
  decltype(begin(m_container)) m_current;
  decltype(end(m_container)) m_end;
};

template <typename Container, typename Char,
          typename Trait = std::char_traits<Char>>
class basic_writeonly_streambuf final
    : public std::basic_streambuf<Char, Trait> {
public:
  Container takeContainer() noexcept { return std::move(m_container); }
  const Container &getContainer() const noexcept { return m_container; }

protected:
  std::streamsize xsputn(const typename Trait::char_type *s,
                         std::streamsize count) override {
    const auto beg = s;
    const auto end = s + count;
    m_container.reserve(m_container.size() + count);
    std::copy(beg, end, std::back_inserter(m_container));
    return count;
  }

  typename Trait::int_type
  overflow(typename Trait::int_type ch = Trait::eof()) override {
    m_container.push_back(Trait::to_char_type(ch));
    return ch;
  }

private:
  Container m_container;
};

template <typename Container>
using writeonly_streambuf = basic_writeonly_streambuf<Container, char>;

template <typename Container>
using readonly_streambuf = basic_readonly_streambuf<Container, char>;

} // namespace ltl
