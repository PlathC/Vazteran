#ifndef VAZTERAN_BACKEND_VULKAN_INSTANCE_HPP
#define VAZTERAN_BACKEND_VULKAN_INSTANCE_HPP

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace vzt
{
	class Instance
	{
	  public:
		Instance(std::string_view name, std::vector<const char*> extensions,
		         const std::vector<const char*>& validationLayers = DefaultValidationLayers);

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;

		Instance(Instance&& other) noexcept;
		Instance& operator=(Instance&& other) noexcept;

		~Instance();

		std::vector<VkPhysicalDevice> enumeratePhysicalDevice();
		std::vector<const char*>      getValidationLayers() const { return m_validationLayers; }

		VkInstance vkHandle() const { return m_handle; };

#ifdef NDEBUG
		static constexpr bool EnableValidationLayers = false;
#else
		static constexpr bool EnableValidationLayers = true;
#endif
		static const std::vector<const char*> DefaultValidationLayers;

	  private:
		static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);

		VkInstance               m_handle         = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
		std::vector<const char*> m_validationLayers;
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	                                                    VkDebugUtilsMessageTypeFlagsEXT             messageType,
	                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	                                                    void*                                       pUserData);
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_INSTANCE_HPP
