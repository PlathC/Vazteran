
#include <cassert>
#include <utility>

#include "Vazteran/Framework/Vulkan/CommandPool.hpp"
#include "Vazteran/Framework/Vulkan/Device.hpp"
#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Framework/Vulkan/RenderPass.hpp"

namespace vzt
{
	CommandPool::CommandPool(Device *logicalDevice, uint32_t bufferCount)
	    : m_logicalDevice(logicalDevice), m_bufferCount(bufferCount)
	{

		vzt::QueueFamilyIndices indices = m_logicalDevice->DeviceQueueFamilyIndices();

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();
		commandPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(m_logicalDevice->VkHandle(), &commandPoolInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	CommandPool::CommandPool(CommandPool &&other) noexcept
	{
		m_logicalDevice  = std::exchange(other.m_logicalDevice, nullptr);
		m_vkHandle       = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		m_commandBuffers = std::exchange(other.m_commandBuffers, {});
		m_bufferCount    = std::exchange(other.m_bufferCount, 0);
	}

	CommandPool &CommandPool::operator=(CommandPool &&other) noexcept
	{
		std::swap(m_logicalDevice, other.m_logicalDevice);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_commandBuffers, other.m_commandBuffers);
		std::swap(m_bufferCount, other.m_bufferCount);

		return *this;
	}

	void CommandPool::AllocateCommandBuffers(uint32_t count)
	{
		if (!m_commandBuffers.empty())
		{
			vkFreeCommandBuffers(m_logicalDevice->VkHandle(), m_vkHandle,
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

		if (vkAllocateCommandBuffers(m_logicalDevice->VkHandle(), &commandBufferAllocInfo, m_commandBuffers.data()) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	}

	void CommandPool::RecordBuffer(uint32_t bufferNumber, Command command) const
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

	VkCommandBuffer &CommandPool::operator[](uint32_t bufferNumber)
	{
		assert(bufferNumber < m_commandBuffers.size() && "bufferNumber should be < than m_commandBuffers.size()");
		return m_commandBuffers[bufferNumber];
	}

	CommandPool::~CommandPool()
	{
		vkFreeCommandBuffers(m_logicalDevice->VkHandle(), m_vkHandle, static_cast<uint32_t>(m_commandBuffers.size()),
		                     m_commandBuffers.data());
		vkDestroyCommandPool(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
	}
} // namespace vzt
