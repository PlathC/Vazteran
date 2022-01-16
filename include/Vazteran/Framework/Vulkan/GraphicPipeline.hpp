#ifndef VAZTERAN_FRAMEWORK_VULKAN_GRAPHICPIPELINE_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_GRAPHICPIPELINE_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "Vazteran/Framework/Vulkan/Descriptor.hpp"
#include "Vazteran/Framework/Vulkan/Shader.hpp"

namespace vzt
{
	class GraphicPipeline;
	class Device;
	class FrameBuffer;
	class ImageView;
	class RenderPass;
	class Sampler;

	enum class DrawType
	{
		Fill  = VK_POLYGON_MODE_FILL,
		Line  = VK_POLYGON_MODE_LINE,
		Point = VK_POLYGON_MODE_POINT
	};

	struct PipelineSettings
	{
		VkFormat              swapChainImageFormat;
		vzt::Size2D<uint32_t> swapChainExtent;
		vzt::DrawType         drawType = DrawType::Fill;
	};

	using BindingDescription   = VkVertexInputBindingDescription;
	using AttributeDescription = std::vector<VkVertexInputAttributeDescription>;

	struct VertexInputDescription
	{
		BindingDescription   binding;
		AttributeDescription attribute;
	};

	class GraphicPipeline
	{
	  public:
		GraphicPipeline(vzt::Device* device, vzt::Program&& program,
		                vzt::VertexInputDescription vertexInputDescription);

		GraphicPipeline(const GraphicPipeline&) = delete;
		GraphicPipeline& operator=(const GraphicPipeline&) = delete;

		GraphicPipeline(GraphicPipeline&& other) noexcept = default;
		GraphicPipeline& operator=(GraphicPipeline&& other) noexcept = default;

		void Bind(VkCommandBuffer commandsBuffer, const vzt::RenderPass* const renderPass);
		void Configure(vzt::PipelineSettings settings);

		const vzt::Program& Program() const { return m_program; };
		VkPipelineLayout    Layout() const { return m_pipelineLayout; }
		VkPipeline          VkHandle() const { return m_vkHandle; }

		~GraphicPipeline();

	  private:
		void Create();
		void Cleanup();

	  private:
		vzt::Device* m_device;

		VkPipeline       m_vkHandle       = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		vzt::VertexInputDescription m_vertexInputDescription;
		vzt::Program                m_program;
		vzt::PipelineSettings       m_settings{};
		vzt::RenderPass const*      m_boundRenderPass = nullptr;
	};
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_GRAPHICPIPELINE_HPP
