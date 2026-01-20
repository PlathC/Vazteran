#ifndef VZT_VULKAN_SETUP_HPP
#define VZT_VULKAN_SETUP_HPP

#include <string_view>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif // VK_NO_PROTOTYPES

#include <vk_mem_alloc.h>
#include <volk.h>

#define VZT_DEFINE_TO_VULKAN_FUNCTION(BaseType, VulkanType) \
    inline constexpr VulkanType toVulkan(const BaseType l) { return static_cast<VulkanType>(l); }

// Helpers
namespace vzt
{
    void vkCheck(VkResult result, std::string_view msg);
} // namespace vzt

#endif // VZT_VULKAN_SETUP_HPP
