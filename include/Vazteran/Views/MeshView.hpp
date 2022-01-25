#ifndef VAZTERAN_BACKEND_VULKAN_MESHVIEW_HPP
#define VAZTERAN_BACKEND_VULKAN_MESHVIEW_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/GpuObjects.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Model.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;

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
		void Configure(vzt::PipelineContextSettings settings);

		void Record(uint32_t imageCount, VkCommandBuffer commandBuffer, const vzt::RenderPass* renderPass);

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
			std::vector<vzt::Texture> textures;
			uint32_t                  descriptorIndex;
		};

		struct ModelDisplayInformation
		{
			const vzt::Model* const modelData;

			vzt::Buffer vertexBuffer;
			vzt::Buffer subMeshesIndexBuffer;

			std::vector<SubMeshData>  subMeshData;
			std::vector<MaterialData> materialsData;
		};

		vzt::DescriptorPool m_descriptorPool;

		vzt::Buffer m_materialInfoBuffer;
		uint32_t    m_materialNb             = 0;
		uint32_t    m_materialInfoOffsetSize = 0;

		vzt::Buffer m_transformBuffer;
		uint32_t    m_transformOffsetSize = 0;

		vzt::SizedDescriptorSet                m_transformDescriptor;
		vzt::SizedDescriptorSet                m_materialDescriptors;
		std::vector<vzt::SamplerDescriptorSet> m_samplersDescriptors;

		std::vector<ModelDisplayInformation> m_models;
		uint32_t                             m_imageCount = 0;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_MESHVIEW_HPP
