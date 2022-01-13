#ifndef VAZTERAN_RENDEROBJECT_HPP
#define VAZTERAN_RENDEROBJECT_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Types.hpp"
#include "Vazteran/Framework/Vulkan/Buffer.hpp"
#include "Vazteran/Framework/Vulkan/GpuObjects.hpp"
#include "Vazteran/Framework/Vulkan/ImageUtils.hpp"

namespace vzt
{
	class GraphicPipeline;
	class LogicalDevice;

	class RenderObject
	{
	  public:
		RenderObject(
		    vzt::LogicalDevice *logicalDevice, vzt::GraphicPipeline *graphicPipeline, vzt::Model *model,
		    uint32_t imageCount);

		RenderObject(const RenderObject &) = delete;
		RenderObject &operator=(const RenderObject &) = delete;

		RenderObject(RenderObject &&other) noexcept = default;
		RenderObject &operator=(RenderObject &&other) noexcept = default;

		vzt::Model *Model() const
		{
			return m_model;
		}

		void Render(VkCommandBuffer commandBuffer, const vzt::GraphicPipeline *graphicPipeline, uint32_t imageCount);
		void UpdateDescriptorSet(
		    VkDescriptorSet descriptorSet, VkBuffer uniformBuffer,
		    const std::unordered_map<uint32_t, vzt::ImageHandler> &textureHandlers);
		void UpdatePushConstants(const vzt::Transforms &objectData);

		~RenderObject();

	  private:
		uint32_t m_imageCount{};
		LogicalDevice *m_logicalDevice = nullptr;
		vzt::Model *m_model;

		std::unique_ptr<vzt::VertexBuffer> m_vertexBuffer;

		struct SubMeshData
		{
			uint32_t materialDataIndex;
			uint32_t minOffset;
			uint32_t maxOffset;
		};
		std::vector<SubMeshData> m_subMeshData;
		std::unique_ptr<vzt::IndexBuffer> m_subMeshesIndexBuffer;
		std::unordered_map<uint32_t, uint32_t> m_uniformRanges;

		struct MaterialData
		{
			std::unordered_map<uint32_t, vzt::ImageHandler> textureHandlers;
			std::vector<VkDescriptorSet> descriptorSets;

			// TODO: Use a single buffer + offset
			std::vector<vzt::Buffer<vzt::MaterialInfo>> uniformBuffers;
		};
		std::vector<std::unique_ptr<MaterialData>> m_materialData;

		std::vector<VkDescriptorPool> m_descriptorPools;
		vzt::Transforms m_currentPushConstants{};
	};
} // namespace vzt

#endif // VAZTERAN_RENDEROBJECT_HPP
