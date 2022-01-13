#ifndef VAZTERAN_COMMANDPOOL_HPP
#define VAZTERAN_COMMANDPOOL_HPP

#include <array>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

#include "Vazteran/Framework/Vulkan/FrameBuffer.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;

	class CommandPool
	{
	  public:
		CommandPool(Device *logicalDevice, uint32_t bufferCount);

		CommandPool(CommandPool &) = delete;
		CommandPool &operator=(const CommandPool &) = delete;

		CommandPool(CommandPool &&other) noexcept;
		CommandPool &operator=(CommandPool &&other) noexcept;

		void AllocateCommandBuffers(uint32_t count);
		void RecordBuffer(
		    const VkExtent2D &swapChainExtent, vzt::GraphicPipeline *graphicPipeline, const FrameBuffer &frameBuffer,
		    uint32_t bufferNumber) const;

		VkCommandBuffer &operator[](uint32_t bufferNumber);

		~CommandPool();

	  private:
		Device *m_logicalDevice;
		VkCommandPool m_vkHandle;
		std::vector<VkCommandBuffer> m_commandBuffers;
		uint32_t m_bufferCount;
	};
} // namespace vzt

#endif // VAZTERAN_COMMANDPOOL_HPP
