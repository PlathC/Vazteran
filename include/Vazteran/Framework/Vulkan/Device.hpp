#ifndef VAZTERAN_PHYSICALDEVICE_HPP
#define VAZTERAN_PHYSICALDEVICE_HPP

#include <functional>
#include <optional>
#include <vector>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

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
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class PhysicalDevice
	{
	  public:
		PhysicalDevice(
		    vzt::Instance *instance, VkSurfaceKHR surface,
		    const std::vector<const char *> &deviceExtensions = vzt::PhysicalDevice::DefaultDeviceExtensions);

		VkPhysicalDevice VkHandle() const
		{
			return m_vkHandle;
		}
		std::vector<const char *> Extensions()
		{
			return m_extensions;
		}
		vzt::SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface) const;
		vzt::QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface) const;
		VkFormat FindDepthFormat();
		VkFormat FindSupportedFormat(
		    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkSampleCountFlagBits MaxUsableSampleCount();

		~PhysicalDevice();

	  private:
		static VkPhysicalDevice FindBestDevice(
		    vzt::Instance *instance, VkSurfaceKHR surface, const std::vector<const char *> &deviceExtensions);
		static const std::vector<const char *> DefaultDeviceExtensions;

		VkPhysicalDevice m_vkHandle;
		std::vector<const char *> m_extensions;
	};

	static bool HasStencilComponent(VkFormat format);

	static bool IsDeviceSuitable(
	    VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char *> &deviceExtensions);
	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &deviceExtensions);
	VkSampleCountFlagBits MaxUsableSampleCount(VkPhysicalDevice physicalDevice);

	class Device
	{
	  public:
		Device(vzt::Instance *instance, VkSurfaceKHR surface);

		Device(const Device &) = delete;
		Device &operator=(const Device &) = delete;

		Device(Device &&) noexcept;
		Device &operator=(Device &&) noexcept;

		void CreateBuffer(
		    VkBuffer &buffer, VmaAllocation &bufferAllocation, VkDeviceSize size, VkBufferUsageFlags usage,
		    VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags preferredFlags = 0);
		void CreateImage(
		    VkImage &image, VmaAllocation &allocation, uint32_t width, uint32_t height, VkFormat format,
		    VkSampleCountFlagBits numSamples, VkImageTiling tiling, VkImageUsageFlags usage);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
		void TransitionImageLayout(
		    VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
		    VkImageAspectFlags aspectFlags);

		using SingleTimeCommandFunction = std::function<void(VkCommandBuffer)>;
		void SingleTimeCommand(const SingleTimeCommandFunction &singleTimeCommandFunction);

		VmaAllocator AllocatorHandle() const
		{
			return m_allocator;
		}
		VkDevice VkHandle() const
		{
			return m_vkHandle;
		}
		VkQueue GraphicsQueue() const
		{
			return m_graphicsQueue;
		}
		VkQueue PresentQueue() const
		{
			return m_presentQueue;
		}
		vzt::QueueFamilyIndices DeviceQueueFamilyIndices() const
		{
			return m_queueFamilyIndices;
		};
		vzt::PhysicalDevice *ChosenPhysicalDevice() const
		{
			return m_physicalDevice.get();
		}

		~Device();

	  private:
		std::unique_ptr<vzt::PhysicalDevice> m_physicalDevice;

		VmaAllocator m_allocator;
		VkDevice m_vkHandle{};
		VkPhysicalDeviceFeatures m_deviceFeatures{};
		VkQueue m_graphicsQueue{};
		VkQueue m_presentQueue{};

		vzt::QueueFamilyIndices m_queueFamilyIndices;
	};
} // namespace vzt

#endif // VAZTERAN_PHYSICALDEVICE_HPP
