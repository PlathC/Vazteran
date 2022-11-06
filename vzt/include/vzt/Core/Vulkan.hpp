#ifndef VZT_CORE_VULKAN_HPP
#define VZT_CORE_VULKAN_HPP

#include <string_view>

// Forward declarations
#ifndef VULKAN_H_

#define NO_SDL_VULKAN_TYPEDEFS

#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || \
    defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T* object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif

#define VK_NULL_HANDLE nullptr
VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDebugUtilsMessengerEXT)
VK_DEFINE_HANDLE(VkPhysicalDevice)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)

enum VkResult : int;

#endif // VULKAN_H_
// Helpers
namespace vzt
{
    void vkCheck(VkResult result, std::string_view msg);
}

#endif // VZT_CORE_VULKAN_HPP
