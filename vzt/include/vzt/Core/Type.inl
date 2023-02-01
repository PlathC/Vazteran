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
    Type* Span<Type>::begin()
    {
        return data;
    }
    template <class Type>
    Type* Span<Type>::end()
    {
        return data + size;
    }
    template <class Type>
    const Type* Span<Type>::begin() const
    {
        return data;
    }
    template <class Type>
    const Type* Span<Type>::end() const
    {
        return data + size;
    }
    template <class Type>
    const Type* Span<Type>::cbegin() const
    {
        return data;
    }
    template <class Type>
    const Type* Span<Type>::cend() const
    {
        return data + size;
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
    const Type* CSpan<Type>::begin() const
    {
        return data;
    }

    template <class Type>
    const Type* CSpan<Type>::end() const
    {
        return data + size;
    }

    template <class Type>
    const Type* CSpan<Type>::cbegin() const
    {
        return data;
    }

    template <class Type>
    const Type* CSpan<Type>::cend() const
    {
        return data + size;
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
    Type* OffsetSpan<Type>::begin()
    {
        return data;
    }

    template <class Type>
    Type* OffsetSpan<Type>::end()
    {
        return data + size;
    }

    template <class Type>
    const Type* OffsetSpan<Type>::begin() const
    {
        return data;
    }

    template <class Type>
    const Type* OffsetSpan<Type>::end() const
    {
        return data + size;
    }

    template <class Type>
    const Type* OffsetSpan<Type>::cbegin() const
    {
        return data;
    }

    template <class Type>
    const Type* OffsetSpan<Type>::cend() const
    {
        return data + size;
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
    const Type* OffsetCSpan<Type>::begin() const
    {
        return data;
    }

    template <class Type>
    const Type* OffsetCSpan<Type>::end() const
    {
        return data + size;
    }

    template <class Type>
    const Type* OffsetCSpan<Type>::cbegin() const
    {
        return data;
    }

    template <class Type>
    const Type* OffsetCSpan<Type>::cend() const
    {
        return data + size;
    }

    template <class Type>
    constexpr Type Range<Type>::size() const
    {
        return end - start;
    }

} // namespace vzt
