#ifndef VAZTERAN_BACKEND_VULKAN_PHYSICAL_DEVICE_HPP
#define VAZTERAN_BACKEND_VULKAN_PHYSICAL_DEVICE_HPP

#include <functional>
#include <optional>
#include <vector>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/ImageTypes.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	class Instance;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
			;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR        capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR>   presentModes;
	};

	class PhysicalDevice
	{
	  public:
		PhysicalDevice(vzt::Instance* instance, VkSurfaceKHR surface,
		               const std::vector<const char*>& deviceExtensions = vzt::PhysicalDevice::DefaultDeviceExtensions);

		~PhysicalDevice();

		std::vector<const char*>     getExtensions() { return m_extensions; }
		vzt::SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface) const;
		VkSampleCountFlagBits        getMaxUsableSampleCount();

		vzt::QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const;
		vzt::Format             findDepthFormat();
		vzt::Format             findSupportedFormat(const std::vector<vzt::Format>& candidates, VkImageTiling tiling,
		                                            VkFormatFeatureFlags features);

		VkPhysicalDevice vkHandle() const { return m_vkHandle; }

	  private:
		static VkPhysicalDevice               findBestDevice(vzt::Instance* instance, VkSurfaceKHR surface,
		                                                     const std::vector<const char*>& deviceExtensions);
		static const std::vector<const char*> DefaultDeviceExtensions;

		VkPhysicalDevice         m_vkHandle;
		std::vector<const char*> m_extensions;
	};

	static bool hasStencilComponent(vzt::Format format);

	static bool                    isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
	                                                const std::vector<const char*>& deviceExtensions);
	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
	VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

	class Device
	{
	  public:
		Device(vzt::Instance* instance, VkSurfaceKHR surface);

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;

		~Device();

		VkBuffer    createBuffer(VmaAllocation& bufferAllocation, VkDeviceSize size, VkBufferUsageFlags usage,
		                         VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags preferredFlags = 0) const;
		VkImage     createImage(VmaAllocation& allocation, uint32_t width, uint32_t height, vzt::Format format,
		                        VkSampleCountFlagBits numSamples, VkImageTiling tiling, vzt::ImageUsage usage) const;
		VkImageView createImageView(VkImage image, vzt::Format format, vzt::ImageAspect aspectFlags) const;

		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
		void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) const;
		void transitionImageLayout(VkImage image, vzt::ImageLayout oldLayout, vzt::ImageLayout newLayout,
		                           vzt::ImageAspect aspectFlags) const;

		using SingleTimeCommandFunction = std::function<void(VkCommandBuffer)>;
		void singleTimeCommand(const SingleTimeCommandFunction& singleTimeCommandFunction) const;

		VmaAllocator            getAllocatorHandle() const { return m_allocator; }
		VkQueue                 getGraphicsQueue() const { return m_graphicsQueue; }
		VkQueue                 getPresentQueue() const { return m_presentQueue; }
		vzt::QueueFamilyIndices getDeviceQueueFamilyIndices() const { return m_queueFamilyIndices; };
		vzt::PhysicalDevice*    getPhysicalDevice() const { return m_physicalDevice.get(); }
		uint64_t                getMinUniformOffsetAlignment() const;

		VkDevice VkHandle() const { return m_vkHandle; }

	  private:
		std::unique_ptr<vzt::PhysicalDevice> m_physicalDevice;

		VmaAllocator             m_allocator = VK_NULL_HANDLE;
		VkDevice                 m_vkHandle  = VK_NULL_HANDLE;
		VkPhysicalDeviceFeatures m_deviceFeatures{};
		VkQueue                  m_graphicsQueue{};
		VkQueue                  m_presentQueue{};

		vzt::QueueFamilyIndices m_queueFamilyIndices;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_PHYSICAL_DEVICE_HPP
