#include <set>
#include <stdexcept>
#include <string>

#define VMA_IMPLEMENTATION

#include "Vazteran/Framework/Vulkan/Device.hpp"
#include "Vazteran/Framework/Vulkan/Instance.hpp"

namespace vzt
{
	const std::vector<const char *> PhysicalDevice::DefaultDeviceExtensions = {
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
	    VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME};

	VkPhysicalDevice PhysicalDevice::FindBestDevice(
	    vzt::Instance *instance, VkSurfaceKHR surface, const std::vector<const char *> &deviceExtensions)
	{
		std::vector<VkPhysicalDevice> physicalDevices = instance->EnumeratePhysicalDevice();
		for (const auto &device : physicalDevices)
		{
			if (IsDeviceSuitable(device, surface, deviceExtensions))
			{
				return device;
			}
		}
		return VK_NULL_HANDLE;
	}

	PhysicalDevice::PhysicalDevice(
	    vzt::Instance *instance, VkSurfaceKHR surface, const std::vector<const char *> &deviceExtensions)
	    : m_vkHandle(FindBestDevice(instance, surface, deviceExtensions)), m_extensions(deviceExtensions)
	{
		if (m_vkHandle == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	SwapChainSupportDetails PhysicalDevice::QuerySwapChainSupport(VkSurfaceKHR surface) const
	{
		return vzt::QuerySwapChainSupport(m_vkHandle, surface);
	}

	QueueFamilyIndices PhysicalDevice::FindQueueFamilies(VkSurfaceKHR surface) const
	{
		return vzt::FindQueueFamilies(m_vkHandle, surface);
	}

	VkFormat PhysicalDevice::FindDepthFormat()
	{
		return FindSupportedFormat(
		    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL,
		    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkFormat PhysicalDevice::FindSupportedFormat(
	    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_vkHandle, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("Failed to find supported format!");
	}

	VkSampleCountFlagBits PhysicalDevice::MaxUsableSampleCount()
	{
		return vzt::MaxUsableSampleCount(m_vkHandle);
	}

	PhysicalDevice::~PhysicalDevice()
	{
	}

	static bool IsDeviceSuitable(
	    VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char *> &deviceExtensions)
	{
		QueueFamilyIndices indices = vzt::FindQueueFamilies(device, surface);

		bool extensionsSupported = vzt::CheckDeviceExtensionSupport(device, deviceExtensions);
		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			vzt::SwapChainSupportDetails swapChainSupport = vzt::QuerySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	static vzt::SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		vzt::SwapChainSupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		vzt::QueueFamilyIndices indices{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		auto queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &deviceExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	static bool HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	static VkSampleCountFlagBits MaxUsableSampleCount(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
		                            physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT)
		{
			return VK_SAMPLE_COUNT_64_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_32_BIT)
		{
			return VK_SAMPLE_COUNT_32_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_16_BIT)
		{
			return VK_SAMPLE_COUNT_16_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_8_BIT)
		{
			return VK_SAMPLE_COUNT_8_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_4_BIT)
		{
			return VK_SAMPLE_COUNT_4_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_2_BIT)
		{
			return VK_SAMPLE_COUNT_2_BIT;
		}

		return VK_SAMPLE_COUNT_1_BIT;
	}

	static constexpr uint32_t GetVulkanApiVersion()
	{
#if VMA_VULKAN_VERSION == 1002000
		return VK_API_VERSION_1_2;
#elif VMA_VULKAN_VERSION == 1001000
		return VK_API_VERSION_1_1;
#elif VMA_VULKAN_VERSION == 1000000
		return VK_API_VERSION_1_0;
#else
#error Invalid VMA_VULKAN_VERSION.
		return UINT32_MAX;
#endif
	}

	Device::Device(vzt::Instance *instance, VkSurfaceKHR surface)
	    : m_physicalDevice(std::make_unique<vzt::PhysicalDevice>(instance, surface))
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		m_queueFamilyIndices = m_physicalDevice->FindQueueFamilies(surface);
		std::set<uint32_t> uniqueQueueFamilies = {
		    m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value()};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		m_deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &m_deviceFeatures;

		auto deviceExtensions = m_physicalDevice->Extensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		auto validationLayers = instance->ValidationLayers();
		if (Instance::EnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice->VkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(m_vkHandle, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_vkHandle, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo = {};

		allocatorInfo.physicalDevice = m_physicalDevice->VkHandle();
		allocatorInfo.device = m_vkHandle;
		allocatorInfo.instance = instance->VkHandle();
		allocatorInfo.vulkanApiVersion = GetVulkanApiVersion();
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

		if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS)
		{
			throw std::runtime_error("Can't create VMA allocator");
		}
	}

	Device::Device(Device &&other) noexcept
	{
		std::swap(other.m_physicalDevice, m_physicalDevice);
		m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		m_allocator = std::exchange(other.m_allocator, static_cast<decltype(m_allocator)>(VK_NULL_HANDLE));
		std::swap(m_deviceFeatures, other.m_deviceFeatures);
		std::swap(m_graphicsQueue, other.m_graphicsQueue);
		std::swap(m_presentQueue, other.m_presentQueue);
		std::swap(m_queueFamilyIndices, other.m_queueFamilyIndices);
	}

	Device &Device::operator=(Device &&other) noexcept
	{
		std::swap(m_physicalDevice, other.m_physicalDevice);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocator, other.m_allocator);
		std::swap(m_deviceFeatures, other.m_deviceFeatures);
		std::swap(m_graphicsQueue, other.m_graphicsQueue);
		std::swap(m_presentQueue, other.m_presentQueue);
		std::swap(m_queueFamilyIndices, other.m_queueFamilyIndices);

		return *this;
	}

	void Device::CreateBuffer(
	    VkBuffer &buffer, VmaAllocation &bufferAllocation, VkDeviceSize size, VkBufferUsageFlags usage,
	    VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags preferredFlags)
	{
		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		// allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		allocInfo.preferredFlags = preferredFlags;

		if (vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &bufferAllocation, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create vertex buffer!");
		}
	}

	void Device::CreateImage(
	    VkImage &image, VmaAllocation &allocation, uint32_t width, uint32_t height, VkFormat format,
	    VkSampleCountFlagBits numSamples, VkImageTiling tiling, VkImageUsageFlags usage)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo imageAllocCreateInfo = {};
		imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (vmaCreateImage(m_allocator, &imageInfo, &imageAllocCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Can't allocate image.");
		}
	}

	VkImageView Device::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(m_vkHandle, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		SingleTimeCommand([&srcBuffer, &dstBuffer, &size](VkCommandBuffer commandBuffer) {
			VkBufferCopy copyRegion{};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		});
	}

	void Device::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height)
	{
		SingleTimeCommand([&srcBuffer, &dstImage, width, height](VkCommandBuffer commandBuffer) {
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = {0, 0, 0};
			region.imageExtent = {width, height, 1};

			vkCmdCopyBufferToImage(
			    commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		});
	}

	void Device::TransitionImageLayout(
	    VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
	    VkImageAspectFlags aspectFlags)
	{
		SingleTimeCommand([&](VkCommandBuffer commandBuffer) {
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = image;
			barrier.subresourceRange.aspectMask = aspectFlags;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (
			    oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			    newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (
			    oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask =
				    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else
			{
				throw std::invalid_argument("Unsupported layout transition!");
			}

			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		});
	}

	void Device::SingleTimeCommand(const vzt::Device::SingleTimeCommandFunction &singleTimeCommandFunction)
	{
		VkCommandPool transferCommandPool;

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		if (vkCreateCommandPool(m_vkHandle, &poolInfo, nullptr, &transferCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = transferCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_vkHandle, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		singleTimeCommandFunction(commandBuffer);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_vkHandle, transferCommandPool, 1, &commandBuffer);
		vkDestroyCommandPool(m_vkHandle, transferCommandPool, nullptr);
	}

	Device::~Device()
	{
		if (m_allocator != VK_NULL_HANDLE)
		{
			vmaDestroyAllocator(m_allocator);
			m_allocator = VK_NULL_HANDLE;
		}

		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyDevice(m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}
} // namespace vzt
