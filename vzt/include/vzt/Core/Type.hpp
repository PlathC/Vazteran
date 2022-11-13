#ifndef VZT_TYPE_HPP
#define VZT_TYPE_HPP

#include <optional>
#include <vector>

namespace vzt
{
    template <class Type>
    class View
    {
      public:
        View() = default;
        View(const Type& data);
        View(const Type* data);

        const Type* operator->() const;
        const Type& operator*() const;
                    operator bool() const;

      private:
        const Type* m_ptr = nullptr;
    };

    template <class Type>
    struct Span
    {
        Type*       data = nullptr;
        std::size_t size = 0;

        Span() = default;
        Span(Type* ptr, std::size_t size);
        Span(const std::vector<Type>& buffer);
    };

    template <class Type>
    struct CSpan
    {
        const Type* data = nullptr;
        std::size_t size = 0;

        CSpan() = default;
        CSpan(Span<Type> span);
        CSpan(Type* ptr, std::size_t size);
        CSpan(const Type* ptr, std::size_t size);
        CSpan(const std::vector<Type>& buffer);
    };

    template <class Type>
    struct OffsetSpan
    {
        Type*       data   = nullptr;
        std::size_t size   = 0;
        std::size_t offset = 0;

        OffsetSpan() = default;
        OffsetSpan(Type* ptr, std::size_t size, std::size_t offset);
    };

    template <class Type>
    using Optional = std::optional<Type>;
} // namespace vzt

#include "vzt/Core/Type.inl"

#endif // VZT_TYPE_HPP
