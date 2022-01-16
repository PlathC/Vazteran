#ifndef VAZTERAN_FRAMEWORK_VULKAN_MESHVIEW_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_MESHVIEW_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Framework/Vulkan/Descriptor.hpp"
#include "Vazteran/Framework/Vulkan/GpuObjects.hpp"
#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;
	class FrameBuffer;

	struct BlinnPhongVertexInput
	{
		vzt::Vec3 position;
		vzt::Vec2 textureCoordinates;
		vzt::Vec3 normal;

		static VkVertexInputBindingDescription                GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
	};

	class MeshView
	{
	  public:
		MeshView(vzt::Device* device, uint32_t imageCount);

		MeshView(const MeshView&) = delete;
		MeshView& operator=(const MeshView&) = delete;

		MeshView(MeshView&& other) noexcept = default;
		MeshView& operator=(MeshView&& other) noexcept = default;

		~MeshView();

		void AddModel(const vzt::Model* const model);
		void Configure(vzt::PipelineSettings settings);
		void Record(uint32_t imageCount, VkCommandBuffer commandBuffer, const vzt::RenderPass* const renderPass);

		void Update(const vzt::Camera& camera);

		vzt::GraphicPipeline* Pipeline() const { return m_graphicPipeline.get(); }

	  private:
		// Rendering objects
		vzt::Device*                          m_device;
		std::unique_ptr<vzt::GraphicPipeline> m_graphicPipeline;
		struct SubMeshData
		{
			uint32_t minOffset;
			uint32_t maxOffset;
			uint32_t materialDataIndex;
		};

		struct MaterialData
		{
			std::vector<std::pair<vzt::ImageView, vzt::Sampler>> textures;
			vzt::DescriptorPool                                  descriptorPool;
		};

		struct ModelDisplayInformation
		{
			const vzt::Model* const modelData;

			vzt::Buffer vertexBuffer;
			vzt::Buffer subMeshesIndexBuffer;

			std::vector<SubMeshData>  subMeshData;
			std::vector<MaterialData> materialsData;
		};

		vzt::Buffer m_materialInfoBuffer;
		uint32_t    m_currentMaterialInfoOffset = 0;
		uint32_t    m_materialInfoOffsetSize    = 0;

		vzt::Buffer m_transformBuffer;
		uint32_t    m_transformOffsetSize = 0;

		vzt::SizedDescriptorSet                m_transformDescriptor;
		vzt::SizedDescriptorSet                m_materialDescriptors;
		std::vector<vzt::SamplerDescriptorSet> m_samplersDescriptors;

		std::vector<ModelDisplayInformation> m_models;
		uint32_t                             m_imageCount = 0;
	};
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_MESHVIEW_HPP
