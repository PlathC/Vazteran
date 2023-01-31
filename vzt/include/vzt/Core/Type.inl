#include "vzt/Core/Type.hpp"

namespace vzt
{
    template <class Type>
    View<Type>::View(const Type* data) : m_ptr(data)
    {
    }

    template <class Type>
    View<Type>::View(const Type& data) : View(&data)
    {
    }

    template <class Type>
    const Type* View<Type>::get() const
    {
        return m_ptr;
    }

    template <class Type>
    const Type* View<Type>::operator->() const
    {
        return m_ptr;
    }

    template <class Type>
    const Type& View<Type>::operator*() const
    {
        return *m_ptr;
    }

    template <class Type>
    View<Type>::operator bool() const
    {
        return m_ptr;
    }

    template <class Type>
    Span<Type>::Span(OffsetSpan<Type> span) : data(span.data), size(span.size)
    {
    }

    template <class Type>
    Span<Type>::Span(const Type& type) : data(&type), size(1)
    {
    }

    template <class Type>
    Span<Type>::Span(Type* ptr, std::size_t size) : data(ptr), size(size)
    {
    }

    template <class Type>
    Span<Type>::Span(const std::vector<Type>& buffer) : data(buffer.data()), size(buffer.size())
    {
    }

    template <class Type>
    template <std::size_t Size>
    Span<Type>::Span(const std::array<Type, Size>& buffer) : data(buffer.data()), size(buffer.size())
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(Span<Type> span) : data(span.data), size(span.size)
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(OffsetSpan<Type> span) : data(span.data), size(span.size)
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(OffsetCSpan<Type> span) : data(span.data), size(span.size)
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(const Type& type) : data(&type), size(1)
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(Type* ptr, std::size_t size) : data(ptr), size(size)
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(const Type* ptr, std::size_t size) : data(ptr), size(size)
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(const std::vector<Type>& buffer) : data(buffer.data()), size(buffer.size())
    {
    }

    template <class Type>
    template <std::size_t Size>
    CSpan<Type>::CSpan(const std::array<Type, Size>& buffer) : data(buffer.data()), size(buffer.size())
    {
    }

    template <class Type>
    OffsetSpan<Type>::OffsetSpan(Span<Type> span) : data(span.data), size(span.size)
    {
    }

    template <class Type>
    OffsetSpan<Type>::OffsetSpan(CSpan<Type> span) : data(span.data), size(span.size)
    {
    }

    template <class Type>
    OffsetSpan<Type>::OffsetSpan(Type* ptr, std::size_t size, std::size_t offset)
        : data(ptr), size(size), offset(offset)
    {
    }

    template <class Type>
    OffsetSpan<Type>::OffsetSpan(Type& ptr, std::size_t size, std::size_t offset)
        : data(&ptr), size(size), offset(offset)
    {
    }

    template <class Type>
    OffsetSpan<Type>::OffsetSpan(std::vector<Type>& buffer, std::size_t offset)
        : data(buffer.data()), size(buffer.size()), offset(offset)
    {
    }

    template <class Type>
    template <std::size_t Size>
    OffsetSpan<Type>::OffsetSpan(std::array<Type, Size>& buffer, std::size_t offset)
        : data(buffer.data()), size(buffer.size()), offset(offset)
    {
    }

    template <class Type>
    OffsetCSpan<Type>::OffsetCSpan(OffsetSpan<Type> span, std::size_t offset)
        : data(span.data), size(span.size), offset(offset)
    {
    }

    template <class Type>
    OffsetCSpan<Type>::OffsetCSpan(const Type* ptr, std::size_t size, std::size_t offset)
        : data(ptr), size(size), offset(offset)
    {
    }
    template <class Type>
    OffsetCSpan<Type>::OffsetCSpan(const Type& ptr, std::size_t size, std::size_t offset)
        : data(&ptr), size(size), offset(offset)
    {
    }

    template <class Type>
    OffsetCSpan<Type>::OffsetCSpan(const std::vector<Type>& buffer, std::size_t offset)
        : data(buffer.data()), size(buffer.size()), offset(offset)
    {
    }

    template <class Type>
    template <std::size_t Size>
    OffsetCSpan<Type>::OffsetCSpan(const std::array<Type, Size>& buffer, std::size_t offset)
        : data(buffer.data()), size(buffer.size()), offset(offset)
    {
    }

    template <class Type>
    constexpr Type Range<Type>::size() const
    {
        return end - start;
    }

} // namespace vzt
