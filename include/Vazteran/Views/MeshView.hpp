#ifndef VAZTERAN_MESHVIEW_HPP
#define VAZTERAN_MESHVIEW_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Core/Type.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Mesh.hpp"
#include "Vazteran/System/Listener.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;

	struct TriangleVertexInput
	{
		vzt::Vec3 position;
		vzt::Vec2 textureCoordinates;
		vzt::Vec3 normal;

		static VertexInputDescription getInputDescription();
	};

	class MeshView
	{
	  public:
		MeshView(Scene& scene);

		MeshView(const MeshView&)            = delete;
		MeshView& operator=(const MeshView&) = delete;

		MeshView(MeshView&& other) noexcept            = default;
		MeshView& operator=(MeshView&& other) noexcept = default;

		~MeshView();

		void configure(const vzt::Device* device, uint32_t imageCount);
		void record(uint32_t imageCount, VkCommandBuffer commandBuffer, GraphicPipeline* pipeline) const;
		void update(const vzt::Camera& camera);

	  private:
		void add(Entity mesh);

		const vzt::Device* m_device;
		uint32_t           m_imageCount = 0;

		vzt::DescriptorPool   m_descriptorPool;
		vzt::DescriptorLayout m_meshDescriptorLayout;

		const std::size_t m_maxSupportedMesh = 128;

		vzt::Buffer m_materialInfoBuffer;
		uint32_t    m_materialNb             = 0;
		uint32_t    m_materialInfoOffsetSize = 0;

		vzt::Buffer m_transformBuffer;
		uint32_t    m_transformNumber     = 0;
		uint32_t    m_transformOffsetSize = 0;

		struct MeshDeviceData
		{
			vzt::Buffer vertexBuffer;
			vzt::Buffer subMeshesIndexBuffer;
			std::size_t transformIndex;

			struct SubMeshData
			{
				uint32_t minOffset;
				uint32_t maxOffset;
				uint32_t materialDataIndex;
			};

			struct TextureData
			{
				vzt::ImageView imageView;
				vzt::Texture   texture;
			};
			std::vector<TextureData> textureData;
			std::vector<SubMeshData> subMeshData;
		};

		struct GenericMaterial
		{
			vzt::Vec4 diffuse; // diffuse + shininess

			static GenericMaterial fromMaterial(const vzt::Material& original);
		};

		struct Transforms
		{
			vzt::Mat4 modelViewMatrix;
			vzt::Mat4 projectionMatrix;
			vzt::Mat4 normalMatrix;
		};

		Scene*         m_scene;
		Listener<Mesh> m_meshListener;
	};

} // namespace vzt

#endif // VAZTERAN_MESHVIEW_HPP
