#ifndef VAZTERAN_BACKEND_VULKAN_GRAPHICPIPELINE_HPP
#define VAZTERAN_BACKEND_VULKAN_GRAPHICPIPELINE_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/Shader.hpp"

namespace vzt
{
	class GraphicPipeline;
	class Device;
	class FrameBuffer;
	class ImageView;
	class RenderPass;
	class Sampler;

	struct PipelineContextSettings
	{
		const vzt::RenderPass* renderPassTemplate;
		vzt::Format            swapChainImageFormat;
		vzt::Size2D<uint32_t>  swapChainExtent;
	};

	using BindingDescription   = VkVertexInputBindingDescription;
	using AttributeDescription = std::vector<VkVertexInputAttributeDescription>;

	struct VertexInputDescription
	{
		BindingDescription   binding;
		AttributeDescription attribute;
	};

	enum class DrawType : uint32_t
	{
		Fill          = VK_POLYGON_MODE_FILL,
		Line          = VK_POLYGON_MODE_LINE,
		Point         = VK_POLYGON_MODE_POINT,
		FillRectangle = VK_POLYGON_MODE_FILL_RECTANGLE_NV
	};

	enum class CullMode : uint32_t
	{
		None         = VK_CULL_MODE_NONE,
		Front        = VK_CULL_MODE_FRONT_BIT,
		Back         = VK_CULL_MODE_BACK_BIT,
		FrontAndBack = VK_CULL_MODE_FRONT_AND_BACK
	};

	enum class FrontFace : uint32_t
	{
		CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		Clockwise        = VK_FRONT_FACE_CLOCKWISE
	};

	struct RasterizationOptions
	{
		vzt::DrawType  drawType  = vzt::DrawType::Fill;
		vzt::CullMode  cullMode  = vzt::CullMode::Back;
		vzt::FrontFace frontFace = vzt::FrontFace::CounterClockwise;
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
		void Configure(vzt::PipelineContextSettings settings);

		vzt::RasterizationOptions& RasterOptions() { return m_rasterOptions; }
		vzt::RasterizationOptions  CRasterOptions() const { return m_rasterOptions; }

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

		vzt::VertexInputDescription  m_vertexInputDescription;
		vzt::Program                 m_program;
		vzt::PipelineContextSettings m_settings{};
		vzt::RasterizationOptions    m_rasterOptions{};
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_GRAPHICPIPELINE_HPP
