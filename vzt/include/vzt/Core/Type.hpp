#ifndef VZT_TYPE_HPP
#define VZT_TYPE_HPP

#include <memory>
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

    template <class HandleType>
    struct HandleSpan
    {
        HandleSpan  data{};
        std::size_t size   = 0;
        std::size_t offset = 0;
    };
} // namespace vzt

#include "vzt/Core/Type.inl"

#endif // VZT_TYPE_HPP
