
#include <cassert>
#include <utility>

#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"

namespace vzt
{
	CommandPool::CommandPool(Device* logicalDevice) : m_logicalDevice(logicalDevice)
	{
		const vzt::QueueFamilyIndices indices = m_logicalDevice->getDeviceQueueFamilyIndices();

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();
		commandPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(m_logicalDevice->vkHandle(), &commandPoolInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	CommandPool::CommandPool(CommandPool&& other) noexcept
	{
		std::swap(m_logicalDevice, other.m_logicalDevice);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_commandBuffers, other.m_commandBuffers);
		std::swap(m_bufferCount, other.m_bufferCount);
	}

	CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
	{
		std::swap(m_logicalDevice, other.m_logicalDevice);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_commandBuffers, other.m_commandBuffers);
		std::swap(m_bufferCount, other.m_bufferCount);

		return *this;
	}

	void CommandPool::allocateCommandBuffers(const uint32_t count)
	{
		if (!m_commandBuffers.empty())
		{
			vkFreeCommandBuffers(m_logicalDevice->vkHandle(), m_vkHandle,
			                     static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
			m_commandBuffers.clear();
		}

		m_bufferCount = count;
		m_commandBuffers.resize(m_bufferCount);

		VkCommandBufferAllocateInfo commandBufferAllocInfo{};
		commandBufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocInfo.commandPool        = m_vkHandle;
		commandBufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_logicalDevice->vkHandle(), &commandBufferAllocInfo, m_commandBuffers.data()) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	}

	void CommandPool::resetBuffers(const uint32_t bufferNumber)
	{
		vkResetCommandBuffer(m_commandBuffers[bufferNumber], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void CommandPool::recordBuffer(const uint32_t bufferNumber, const Command command) const
	{
		assert(bufferNumber < m_commandBuffers.size() && "bufferNumber should be less than m_commandBuffers.size()");

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags            = 0;       // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(m_commandBuffers[bufferNumber], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to start the recording of the command buffer!");
		}

		command(m_commandBuffers[bufferNumber]);

		if (vkEndCommandBuffer(m_commandBuffers[bufferNumber]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record the command buffer!");
		}
	}

	VkCommandBuffer& CommandPool::operator[](uint32_t bufferNumber)
	{
		assert(bufferNumber < m_commandBuffers.size() && "bufferNumber should be < than m_commandBuffers.size()");
		return m_commandBuffers[bufferNumber];
	}

	CommandPool::~CommandPool()
	{
		if (!m_commandBuffers.empty())
		{
			vkFreeCommandBuffers(m_logicalDevice->vkHandle(), m_vkHandle,
			                     static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		}

		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicalDevice->vkHandle(), m_vkHandle, nullptr);
		}
	}
} // namespace vzt
