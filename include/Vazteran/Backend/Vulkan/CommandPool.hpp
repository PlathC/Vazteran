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

		~CommandPool();

		VkCommandBuffer& operator[](uint32_t bufferNumber);

		void allocateCommandBuffers(uint32_t count);
		void resetBuffers(uint32_t bufferNumber);
		void recordBuffer(uint32_t bufferNumber, Command command) const;

	  private:
		Device*                      m_logicalDevice;
		VkCommandPool                m_vkHandle{};
		std::vector<VkCommandBuffer> m_commandBuffers;
		uint32_t                     m_bufferCount{};
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_COMMAND_POOL_HPP
