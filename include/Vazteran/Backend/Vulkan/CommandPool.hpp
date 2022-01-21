#ifndef VAZTERAN_BACKEND_VULKAN_COMMAND_POOL_HPP
#define VAZTERAN_BACKEND_VULKAN_COMMAND_POOL_HPP

#include <array>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;

	using Command = std::function<void(VkCommandBuffer /*commandBuffer*/)>;

	class CommandPool
	{
	  public:
		CommandPool(Device* logicalDevice);

		CommandPool(CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;

		CommandPool(CommandPool&& other) noexcept;
		CommandPool& operator=(CommandPool&& other) noexcept;

		void AllocateCommandBuffers(uint32_t count);
		void ResetBuffers(uint32_t bufferNumber);
		void RecordBuffer(uint32_t bufferNumber, Command command) const;

		VkCommandBuffer& operator[](uint32_t bufferNumber);

		~CommandPool();

	  private:
		Device*                      m_logicalDevice;
		VkCommandPool                m_vkHandle;
		std::vector<VkCommandBuffer> m_commandBuffers;
		uint32_t                     m_bufferCount;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_COMMAND_POOL_HPP
