#pragma once

#include <iterator>
#include <streambuf>

#include "algos.h"
#include "operator.h"
#include "traits.h"

namespace ltl {
using std::begin;
using std::end;

template <typename Container, typename Char, typename Trait = std::char_traits<Char>>
class basic_readonly_streambuf final : public std::basic_streambuf<Char, Trait> {
    using T = std::decay_t<decltype(*begin(std::declval<Container &>()))>;
    using char_type = typename Trait::char_type;
    using int_type = typename Trait::int_type;
    using pos_type = typename Trait::pos_type;
    using off_type = typename Trait::off_type;

  public:
    basic_readonly_streambuf(Container container) noexcept : m_container{std::move(container)} { computeIterators(); }

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

    void clear() {
        m_container = Container{};
        computeIterators();
    }

  protected:
    std::streamsize showmanyc() override { return std::distance(m_current, m_end); }

    std::streamsize xsgetn(char_type *s, std::streamsize count) override {
        if (showmanyc() < count)
            return 0;
        auto to_char = map([](auto x) { return static_cast<char_type>(x); });
        ltl::copy(Range{m_current, m_end} | to_char | take_n(count), s);
        std::advance(m_current, count);
        return count;
    }

    int_type underflow() override {
        if (m_current != m_end) {
            return Trait::to_int_type(static_cast<char_type>(*m_current));
        } else {
            return Trait::eof();
        }
    }

    int_type uflow() override {
        if (m_current != m_end) {
            auto result = underflow();
            ++m_current;
            return result;
        } else {
            return Trait::eof();
        }
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode = std::ios_base::in) override {
        std::size_t offset;
        if (dir == std::ios_base::cur)
            offset = std::distance(m_begin, m_current) + off;
        else if (dir == std::ios_base::beg)
            offset = off;
        else
            offset = m_container.size() + off;
        m_current = std::next(m_begin, offset);
        return offset;
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode = std::ios_base::in) override {
        return seekoff(pos, std::ios_base::beg);
    }

    int sync() override {
        auto is_erasable = IS_VALID((c), c.erase(m_begin, m_current));

        if_constexpr(is_erasable(m_container)) { m_container.erase(m_begin, m_current); }
        else {
            Container newContainer;
            newContainer.reserve(std::distance(m_current, m_end));
            std::copy(m_current, m_end, std::back_inserter(newContainer));
            m_container = std::move(newContainer);
        }
        computeIterators();
        return 0;
    }

    int_type pbackfail(int_type c = Trait::eof()) override {
        --m_current;
        if (c != Trait::eof())
            *m_current = static_cast<T>(Trait::to_char_type(c));
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

template <typename Container, typename Char, typename Trait = std::char_traits<Char>>
class basic_writeonly_streambuf final : public std::basic_streambuf<Char, Trait> {
    using T = std::decay_t<decltype(*begin(std::declval<Container &>()))>;
    using char_type = typename Trait::char_type;
    using int_type = typename Trait::int_type;

  public:
    Container takeContainer() noexcept { return std::move(m_container); }
    const Container &getContainer() const noexcept { return m_container; }

  protected:
    std::streamsize xsputn(const typename Trait::char_type *s, std::streamsize count) override {
        auto to_underlying_type = map([](auto x) { return static_cast<T>(x); });
        m_container.reserve(m_container.size() + count);
        ltl::copy(Range{s, s + count} | to_underlying_type, std::back_inserter(m_container));
        return count;
    }

    int_type overflow(int_type ch = Trait::eof()) override {
        if (ch != Trait::eof())
            m_container.push_back(static_cast<T>(Trait::to_char_type(ch)));
        return 0;
    }

  private:
    Container m_container;
};

template <typename Container>
using writeonly_streambuf = basic_writeonly_streambuf<Container, char>;

template <typename Container>
using readonly_streambuf = basic_readonly_streambuf<Container, char>;

struct as_byte {
    char *begin;
    std::size_t count;

    template <typename T>
    as_byte(T &&t) {
        begin = reinterpret_cast<char *>(const_cast<std::decay_t<T> *>(std::addressof(t)));
        count = sizeof(std::decay_t<T>);
    }
};

inline std::istream &operator>>(std::istream &stream, as_byte b) {
    stream.read(b.begin, b.count);
    return stream;
}

inline std::ostream &operator<<(std::ostream &stream, as_byte b) {
    stream.write(b.begin, b.count);
    return stream;
}

} // namespace ltl
