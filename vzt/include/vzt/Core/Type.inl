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
    Span<Type>::Span(const Type& type) : data(&type), size(1)
    {
    }

    template <class Type>
    Span<Type>::Span(Type* ptr, std::size_t size) : data(ptr), size(size)
    {
    }

    template <class Type>
    Span<Type>::Span(std::vector<Type>& buffer, std::size_t offset)
        : data(buffer.data() + offset), size(buffer.size() - offset)
    {
    }

    template <class Type>
    template <std::size_t Size>
    Span<Type>::Span(std::array<Type, Size>& buffer, std::size_t offset)
        : data(buffer.data() + offset), size(buffer.size() - offset)
    {
    }

    template <class Type>
    Type& Span<Type>::operator[](std::size_t i)
    {
        return *(data + i);
    }
    template <class Type>
    const Type& Span<Type>::operator[](std::size_t i) const
    {
        return *(data + i);
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
    CSpan<Type>::CSpan(Span<Type> span, std::size_t offset) : data(span.data + offset), size(span.size - offset)
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
    CSpan<Type>::CSpan(const std::vector<Type>& buffer, std::size_t offset)
        : data(buffer.data() + offset), size(buffer.size() - offset)
    {
    }

    template <class Type>
    template <std::size_t Size>
    CSpan<Type>::CSpan(const std::array<Type, Size>& buffer) : data(buffer.data()), size(buffer.size())
    {
    }

    template <class Type>
    const Type& CSpan<Type>::operator[](std::size_t i) const
    {
        return *(data + i);
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
    constexpr Type Range<Type>::size() const
    {
        return end - start;
    }

} // namespace vzt
