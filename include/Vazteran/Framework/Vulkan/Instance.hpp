#ifndef VAZTERAN_FRAMEWORK_VULKAN_INSTANCE_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_INSTANCE_HPP

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace vzt
{
	class Instance
	{
	  public:
		Instance(
		    std::string_view name, std::vector<const char *> extensions,
		    const std::vector<const char *> &validationLayers = DefaultValidationLayers);

		Instance(const Instance &) = delete;
		Instance &operator=(const Instance &) = delete;

		Instance(Instance &&other) noexcept;
		Instance &operator=(Instance &&other) noexcept;

		std::vector<VkPhysicalDevice> EnumeratePhysicalDevice();
		VkInstance VkHandle() const
		{
			return m_handle;
		};
		std::vector<const char *> ValidationLayers() const
		{
			return m_validationLayers;
		}

		~Instance();

#ifdef NDEBUG
		static constexpr bool EnableValidationLayers = false;
#else
		static constexpr bool EnableValidationLayers = true;
#endif
		static const std::vector<const char *> DefaultValidationLayers;

	  private:
		static bool CheckValidationLayerSupport(const std::vector<const char *> &validationLayers);

		VkInstance m_handle = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
		std::vector<const char *> m_validationLayers;
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_INSTANCE_HPP
