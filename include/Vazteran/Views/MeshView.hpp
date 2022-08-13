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
#include "Vazteran/System/Scene.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;

	struct TriangleVertexInput
	{
		Vec3 position;
		Vec2 textureCoordinates;
		Vec3 normal;

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

		void configure(const Device* device, uint32_t imageCount);
		void record(uint32_t imageCount, VkCommandBuffer commandBuffer, GraphicPipeline* pipeline) const;
		void update(const Entity& cameraEntity);

	  private:
		void add(entt::registry& registry, entt::entity entity);

		const Device* m_device;
		uint32_t      m_imageCount = 0;

		DescriptorPool   m_descriptorPool;
		DescriptorLayout m_meshDescriptorLayout;

		const std::size_t m_maxSupportedMesh = 128;

		Buffer   m_materialInfoBuffer;
		uint32_t m_materialNb             = 0;
		uint32_t m_materialInfoOffsetSize = 0;

		Buffer   m_transformBuffer;
		uint32_t m_transformNumber     = 0;
		uint32_t m_transformOffsetSize = 0;

		struct MeshDeviceData
		{
			Buffer      vertexBuffer;
			Buffer      subMeshesIndexBuffer;
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
			Vec4 diffuse; // diffuse + shininess

			static GenericMaterial fromMaterial(const Material& original);
		};

		struct Transforms
		{
			Mat4 modelViewMatrix;
			Mat4 projectionMatrix;
			Mat4 normalMatrix;
		};

		Scene*     m_scene;
		Connection m_connection;
		//  Listener<Mesh> m_meshListener;
	};

} // namespace vzt

#endif // VAZTERAN_MESHVIEW_HPP
