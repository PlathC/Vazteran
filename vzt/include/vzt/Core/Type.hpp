#ifndef VZT_TYPE_HPP
#define VZT_TYPE_HPP

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
} // namespace vzt

#include "vzt/Core/Type.inl"

#endif // VZT_TYPE_HPP
