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

		bool IsComplete()
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

		VkPhysicalDevice             VkHandle() const { return m_vkHandle; }
		std::vector<const char*>     Extensions() { return m_extensions; }
		vzt::SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface) const;
		vzt::QueueFamilyIndices      FindQueueFamilies(VkSurfaceKHR surface) const;
		vzt::Format                  FindDepthFormat();
		vzt::Format           FindSupportedFormat(const std::vector<vzt::Format>& candidates, VkImageTiling tiling,
		                                          VkFormatFeatureFlags features);
		VkSampleCountFlagBits MaxUsableSampleCount();

		~PhysicalDevice();

	  private:
		static VkPhysicalDevice               FindBestDevice(vzt::Instance* instance, VkSurfaceKHR surface,
		                                                     const std::vector<const char*>& deviceExtensions);
		static const std::vector<const char*> DefaultDeviceExtensions;

		VkPhysicalDevice         m_vkHandle;
		std::vector<const char*> m_extensions;
	};

	static bool HasStencilComponent(vzt::Format format);

	static bool                    IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
	                                                const std::vector<const char*>& deviceExtensions);
	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static QueueFamilyIndices      FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
	VkSampleCountFlagBits MaxUsableSampleCount(VkPhysicalDevice physicalDevice);

	class Device
	{
	  public:
		Device(vzt::Instance* instance, VkSurfaceKHR surface);

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;

		~Device();

		VkBuffer    CreateBuffer(VmaAllocation& bufferAllocation, VkDeviceSize size, VkBufferUsageFlags usage,
		                         VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags preferredFlags = 0);
		VkImage     CreateImage(VmaAllocation& allocation, uint32_t width, uint32_t height, vzt::Format format,
		                        VkSampleCountFlagBits numSamples, VkImageTiling tiling, vzt::ImageUsage usage);
		VkImageView CreateImageView(VkImage image, vzt::Format format, vzt::ImageAspect aspectFlags);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
		void TransitionImageLayout(VkImage image, vzt::ImageLayout oldLayout, vzt::ImageLayout newLayout,
		                           vzt::ImageAspect aspectFlags);

		using SingleTimeCommandFunction = std::function<void(VkCommandBuffer)>;
		void SingleTimeCommand(const SingleTimeCommandFunction& singleTimeCommandFunction);

		VmaAllocator            AllocatorHandle() const { return m_allocator; }
		VkDevice                VkHandle() const { return m_vkHandle; }
		VkQueue                 GraphicsQueue() const { return m_graphicsQueue; }
		VkQueue                 PresentQueue() const { return m_presentQueue; }
		vzt::QueueFamilyIndices DeviceQueueFamilyIndices() const { return m_queueFamilyIndices; };
		vzt::PhysicalDevice*    ChosenPhysicalDevice() const { return m_physicalDevice.get(); }

		uint64_t MinUniformOffsetAligment() const;

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
