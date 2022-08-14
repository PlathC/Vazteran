#ifndef VAZTERAN_VIEW_HPP
#define VAZTERAN_VIEW_HPP

#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"

namespace vzt
{
	class Device;

	class View
	{
	  public:
		View(uint32_t imageNb) : m_imageNb(imageNb) {}
		virtual ~View() = default;

		virtual void refresh()                                                              = 0;
		virtual void configure(const PipelineContextSettings& settings)                     = 0;
		virtual void record(uint32_t imageId, VkCommandBuffer cmd,
		                    const std::vector<VkDescriptorSet>& engineDescriptorSets) const = 0;

	  protected:
		uint32_t m_imageNb;
	};
} // namespace vzt

#endif // VAZTERAN_VIEW_HPP
