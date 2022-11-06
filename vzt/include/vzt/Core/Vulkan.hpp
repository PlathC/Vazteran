#ifndef VZT_CORE_VULKAN_HPP
#define VZT_CORE_VULKAN_HPP

#include <string_view>

// Forward declarations
#define VZT_DEFINE_VKHANDLE(object) typedef struct object##_T* object;

#define VK_NULL_HANDLE nullptr
VZT_DEFINE_VKHANDLE(VkInstance)
VZT_DEFINE_VKHANDLE(VkDebugUtilsMessengerEXT)

enum VkResult : int;

// Helpers
namespace vzt
{
    void vkCheck(VkResult result, std::string_view msg);
}

#endif // VZT_CORE_VULKAN_HPP
