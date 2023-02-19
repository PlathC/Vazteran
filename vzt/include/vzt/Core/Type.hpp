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

        const Type* get() const;

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
        Span(std::vector<Type>& buffer, std::size_t offset = 0);

        template <std::size_t Size>
        Span(std::array<Type, Size>& buffer, std::size_t offset = 0);

        Type&       operator[](std::size_t i);
        const Type& operator[](std::size_t i) const;

        Type*       begin();
        Type*       end();
        const Type* begin() const;
        const Type* end() const;
    };

    template <class Type>
    struct CSpan
    {
        const Type* data = nullptr;
        std::size_t size = 0;

        CSpan() = default;
        CSpan(Span<Type> span, std::size_t offset = 0);
        CSpan(const Type& span);
        CSpan(const std::vector<Type>& buffer, std::size_t offset = 0);
        CSpan(Type* ptr, std::size_t size);
        CSpan(const Type* ptr, std::size_t size);

        template <std::size_t Size>
        CSpan(const std::array<Type, Size>& buffer);

        const Type& operator[](std::size_t i) const;

        const Type* begin() const;
        const Type* end() const;
    };

    template <class Type>
    struct StridedSpan
    {
        Type        data;
        std::size_t size;
        std::size_t stride;
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
