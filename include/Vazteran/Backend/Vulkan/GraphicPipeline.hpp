#ifndef VAZTERAN_VULKAN_GRAPHICPIPELINE_HPP
#define VAZTERAN_VULKAN_GRAPHICPIPELINE_HPP

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

	struct PipelineContextSettings
	{
		const Device*           device;
		const RenderPass*       renderPassTemplate;
		const DescriptorLayout* engineDescriptors;
		uint32_t                attachmentCount;
		Uvec2                   targetSize;
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
		GraphicPipeline() = default;

		GraphicPipeline(Program&& program, Optional<VertexInputDescription> vertexInputDescription = {});
		GraphicPipeline(Program&& program, DescriptorLayout userDefinedDescriptorLayout,
		                Optional<VertexInputDescription> vertexInputDescription = {});

		GraphicPipeline(const GraphicPipeline&)            = delete;
		GraphicPipeline& operator=(const GraphicPipeline&) = delete;

		GraphicPipeline(GraphicPipeline&& other) noexcept;
		GraphicPipeline& operator=(GraphicPipeline&& other) noexcept;

		~GraphicPipeline();

		void configure(vzt::PipelineContextSettings settings);
		void bind(VkCommandBuffer commandsBuffer) const;
		void bind(VkCommandBuffer commandsBuffer, const std::vector<VkDescriptorSet>& descriptorSets) const;

		vzt::RasterizationOptions& getRasterOptions() { return m_rasterOptions; }
		vzt::RasterizationOptions  getCRasterOptions() const { return m_rasterOptions; }

		const vzt::Program& getProgram() const { return m_program; };
		VkPipelineLayout    layout() const { return m_pipelineLayout; }

		VkPipeline vkHandle() const { return m_vkHandle; }

	  private:
		void create();
		void cleanup();

	  private:
		VkPipeline       m_vkHandle       = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		Program                          m_program;
		Optional<DescriptorLayout>       m_userDefinedDescriptorLayout;
		Optional<VertexInputDescription> m_vertexInputDescription;
		PipelineContextSettings          m_contextSettings{};
		RasterizationOptions             m_rasterOptions{};
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_GRAPHICPIPELINE_HPP
