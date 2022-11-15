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
    Span<Type>::Span(Type* ptr, std::size_t size) : data(ptr), size(size)
    {
    }

    template <class Type>
    Span<Type>::Span(const std::vector<Type>& buffer) : data(buffer.data()), size(buffer.size())
    {
    }

    template <class Type>
    CSpan<Type>::CSpan(Span<Type> span) : data(span.data), size(span.size)
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
    OffsetSpan<Type>::OffsetSpan(Type* ptr, std::size_t size, std::size_t offset)
        : data(ptr), size(size), offset(offset)
    {
    }

    template <class Type>
    OffsetSpan<Type>::OffsetSpan(Type& ptr, std::size_t size, std::size_t offset)
        : data(&ptr), size(size), offset(offset)
    {
    }
} // namespace vzt
