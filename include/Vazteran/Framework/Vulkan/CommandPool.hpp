#ifndef VAZTERAN_FRAMEWORK_VULKAN_COMMAND_POOL_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_COMMAND_POOL_HPP

#include <array>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

#include "Vazteran/Framework/Vulkan/FrameBuffer.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;

	using Command = std::function<void(VkCommandBuffer /*commandBuffer*/)>;

	class CommandPool
	{
	  public:
		CommandPool(Device *logicalDevice, uint32_t bufferCount);

		CommandPool(CommandPool &) = delete;
		CommandPool &operator=(const CommandPool &) = delete;

		CommandPool(CommandPool &&other) noexcept;
		CommandPool &operator=(CommandPool &&other) noexcept;

		void AllocateCommandBuffers(uint32_t count);
		void RecordBuffer(uint32_t bufferNumber, Command command) const;

		VkCommandBuffer &operator[](uint32_t bufferNumber);

		~CommandPool();

	  private:
		Device                      *m_logicalDevice;
		VkCommandPool                m_vkHandle;
		std::vector<VkCommandBuffer> m_commandBuffers;
		uint32_t                     m_bufferCount;
	};
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_COMMAND_POOL_HPP
