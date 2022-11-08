#ifndef VZT_CORE_VULKAN_HPP
#define VZT_CORE_VULKAN_HPP

#include <unordered_map>

#include <vulkan/vulkan_core.h>

#define VZT_DEFINE_TO_VULKAN_FUNCTION(BaseType, VulkanType) \
    inline constexpr VulkanType toVulkan(const BaseType l)  \
    {                                                       \
        return static_cast<VulkanType>(l);                  \
    }

// Helpers
namespace vzt
{
    void     vkCheck(VkResult result, std::string_view msg);
    uint32_t getAPIVersion();
} // namespace vzt

#endif // VZT_CORE_VULKAN_HPP
