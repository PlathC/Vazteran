#ifndef VZT_TYPE_HPP
#define VZT_TYPE_HPP

#include <array>
#include <limits>
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
        Span(const Type& type);
        Span(Type* ptr, std::size_t size);
        Span(const std::vector<Type>& buffer);

        template <std::size_t Size>
        Span(const std::array<Type, Size>& buffer);
    };

    template <class Type>
    struct CSpan
    {
        const Type* data = nullptr;
        std::size_t size = 0;

        CSpan() = default;
        CSpan(Span<Type> span);
        CSpan(const Type& span);
        CSpan(Type* ptr, std::size_t size);
        CSpan(const Type* ptr, std::size_t size);
        CSpan(const std::vector<Type>& buffer);

        template <std::size_t Size>
        CSpan(const std::array<Type, Size>& buffer);
    };

    template <class Type>
    struct OffsetSpan
    {
        Type*       data   = nullptr;
        std::size_t size   = 0;
        std::size_t offset = 0;

        OffsetSpan() = default;
        OffsetSpan(Type* ptr, std::size_t size, std::size_t offset = 0u);
        OffsetSpan(Type& ptr, std::size_t size, std::size_t offset = 0u);
    };

    template <class Type>
    using Optional = std::optional<Type>;

    template <class Type = std::size_t>
    struct Range
    {
        static_assert(std::numeric_limits<Type>::is_integer, "Type must be an integer type");

        Type start;
        Type end;

        constexpr Type size() const;
    };

    // From https://en.cppreference.com/w/cpp/utility/variant/visit
    template <class... Ts>
    struct Overloaded : Ts...
    {
        using Ts::operator()...;
    };

    template <class... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;
} // namespace vzt

#include "vzt/Core/Type.inl"

#endif // VZT_TYPE_HPP
