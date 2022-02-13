#include <iostream>
#include <stdexcept>

#include "Vazteran/Backend/Vulkan/Instance.hpp"

namespace vzt
{
	const std::vector<const char*> Instance::DefaultValidationLayers = {"VK_LAYER_KHRONOS_validation"};

	Instance::Instance(std::string_view name, std::vector<const char*> extensions,
	                   const std::vector<const char*>& validationLayers)
	    : m_validationLayers(validationLayers)
	{
		if (EnableValidationLayers && !checkValidationLayerSupport(m_validationLayers))
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName   = std::string(name).c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
		appInfo.pEngineName        = "No Engine";
		appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion         = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		if constexpr (EnableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		auto createInfoDebugMessenger = [](VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
			createInfo                 = {};
			createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
		};

		if constexpr (EnableValidationLayers)
		{
			createInfo.enabledLayerCount   = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();

			VkDebugUtilsMessengerCreateInfoEXT instanceDebugCreateInfo;
			createInfoDebugMessenger(instanceDebugCreateInfo);
			createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&instanceDebugCreateInfo);
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance!");
		}

		if constexpr (!EnableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
		createInfoDebugMessenger(debugMessengerCreateInfo);

		auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr(m_handle, "vkCreateDebugUtilsMessengerEXT"));
		if (vkCreateDebugUtilsMessengerEXT == nullptr ||
		    vkCreateDebugUtilsMessengerEXT(m_handle, &debugMessengerCreateInfo, nullptr, &m_debugMessenger) !=
		        VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger!");
		}
	}

	Instance::Instance(Instance&& other) noexcept
	{
		m_handle = std::exchange(other.m_handle, static_cast<decltype(m_handle)>(VK_NULL_HANDLE));
		m_debugMessenger =
		    std::exchange(other.m_debugMessenger, static_cast<decltype(m_debugMessenger)>(VK_NULL_HANDLE));
		m_validationLayers = std::exchange(other.m_validationLayers, {});
	}

	Instance& Instance::operator=(Instance&& other) noexcept
	{
		std::swap(m_handle, other.m_handle);
		std::swap(m_debugMessenger, other.m_debugMessenger);
		std::swap(m_validationLayers, other.m_validationLayers);

		return *this;
	}

	std::vector<VkPhysicalDevice> Instance::enumeratePhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_handle, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		auto devices = std::vector<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(m_handle, &deviceCount, devices.data());

		return devices;
	}

	Instance::~Instance()
	{
		auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr(m_handle, "vkDestroyDebugUtilsMessengerEXT"));
		if (vkDestroyDebugUtilsMessengerEXT != nullptr && m_handle != VK_NULL_HANDLE &&
		    m_debugMessenger != VK_NULL_HANDLE)
		{
			vkDestroyDebugUtilsMessengerEXT(m_handle, m_debugMessenger, nullptr);
		}

		if (m_handle != VK_NULL_HANDLE)
		{
			vkDestroyInstance(m_handle, nullptr);
		}
	}

	bool Instance::checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const std::string& layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (layerName == layerProperties.layerName)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	                                                    VkDebugUtilsMessageTypeFlagsEXT             messageType,
	                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	                                                    void*                                       pUserData)
	{

		// TODO: Handle this in a real logger
		std::string severityDisplay;
		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			severityDisplay = "VERBOSE";
		}
		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			severityDisplay = "INFO";
		}
		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			severityDisplay = "WARNING";
		}
		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			severityDisplay = "ERROR";
		}
		std::cerr << "[" << severityDisplay << "] "
		          << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
} // namespace vzt
