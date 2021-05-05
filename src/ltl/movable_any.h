#pragma once

#include <cassert>
#include <memory>
#include <typeindex>
#include <utility>

namespace ltl {
class movable_any {
    struct Concept {
        virtual const void *ptr() const noexcept = 0;
        virtual void *ptr() noexcept = 0;
        virtual ~Concept() = default;
    };

    template <typename T>
    class Model : public Concept {
      public:
        Model(T &&v) : m_underlying{std::move(v)} {}

        void *ptr() noexcept override { return std::addressof(m_underlying); }
        const void *ptr() const noexcept override { return std::addressof(m_underlying); }

      private:
        T m_underlying;
    };

  public:
    template <typename T>
    movable_any(T t) noexcept :
        m_typeIndex{typeid(T)}, //
        m_concept{std::make_shared<Model<T>>(std::move(t))} {}

    template <typename T>
    T &get() noexcept {
        assert(m_typeIndex == typeid(T));
        return *static_cast<T *>(m_concept->ptr());
    }

    template <typename T>
    const T &get() const noexcept {
        assert(m_typeIndex == typeid(T));
        return *static_cast<const T *>(m_concept->ptr());
    }

    std::type_index type() const noexcept { return m_typeIndex; }

  private:
    std::type_index m_typeIndex;
    std::shared_ptr<Concept> m_concept;
};

} // namespace ltl
