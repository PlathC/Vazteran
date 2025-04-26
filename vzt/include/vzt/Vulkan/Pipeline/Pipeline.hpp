#ifndef VZT_VULKAN_PIPELINE_PIPEINE_HPP
#define VZT_VULKAN_PIPELINE_PIPEINE_HPP

#include "vzt/Vulkan/Descriptor.hpp"

namespace vzt
{
    class Pipeline
    {
      public:
        Pipeline() = default;
        inline Pipeline(Pipeline&& other) noexcept;
        inline Pipeline& operator=(Pipeline&& other) noexcept;

        virtual ~Pipeline() = default;
        inline const DescriptorLayout& getDescriptorLayout() const;

      protected:
        DescriptorLayout m_descriptorLayout;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline/Pipeline.inl"

#endif // VZT_VULKAN_PIPELINE_PIPEINE_HPP