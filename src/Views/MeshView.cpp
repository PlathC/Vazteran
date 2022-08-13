#include "Vazteran/Views/MeshView.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Data/Transform.hpp"

namespace vzt
{

	VertexInputDescription TriangleVertexInput::getInputDescription()
	{
		constexpr VkVertexInputBindingDescription bindingDescription{0, sizeof(vzt::TriangleVertexInput),
		                                                             VK_VERTEX_INPUT_RATE_VERTEX};

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
		    std::vector<VkVertexInputAttributeDescription>(3);
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset   = offsetof(vzt::TriangleVertexInput, position);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset   = offsetof(vzt::TriangleVertexInput, textureCoordinates);

		attributeDescriptions[2].binding  = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset   = offsetof(vzt::TriangleVertexInput, normal);

		return vzt::VertexInputDescription{bindingDescription, std::move(attributeDescriptions)};
	}

	MeshView::GenericMaterial MeshView::GenericMaterial::fromMaterial(const Material& original)
	{
		return GenericMaterial{glm::vec4(glm::vec3(original.color), original.shininess)};
	}

	MeshView::MeshView(Scene& scene) : m_scene(&scene)
	{
		m_meshDescriptorLayout = vzt::DescriptorLayout();
		m_meshDescriptorLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
		m_meshDescriptorLayout.addBinding(1, vzt::DescriptorType::UniformBuffer);
		m_meshDescriptorLayout.addBinding(2, vzt::DescriptorType::CombinedSampler);
	}

	MeshView::~MeshView() {}

	void MeshView::add(entt::registry& registry, entt::entity entity)
	{
		Entity      meshEntity = {registry, entity};
		const Mesh& mesh       = meshEntity.get<Mesh>();

		const auto& vertices = mesh.vertices;
		const auto& normals  = mesh.normals;
		const auto& uvs      = mesh.uvs;

		std::vector<TriangleVertexInput> verticesData;
		verticesData.reserve(vertices.size());
		for (std::size_t i = 0; i < vertices.size(); i++)
		{
			verticesData.emplace_back(TriangleVertexInput{vertices[i], uvs[i], normals[i]});
		}

		MeshDeviceData& meshData = meshEntity.emplace<MeshDeviceData>();

		meshData.vertexBuffer   = Buffer(m_device, Span<TriangleVertexInput>(verticesData), BufferUsage::VertexBuffer);
		meshData.transformIndex = m_transformNumber;
		auto subMeshRawIndices  = mesh.getVertexIndices();
		auto subMeshMaterialIndices = mesh.getMaterialIndices();

		meshData.subMeshData.reserve(subMeshRawIndices.size());
		std::vector<uint32_t> submeshIndices;
		submeshIndices.reserve(subMeshRawIndices.size());
		for (std::size_t i = 0; i < subMeshRawIndices.size(); i++)
		{
			MeshDeviceData::SubMeshData data;
			data.minOffset         = static_cast<uint32_t>(submeshIndices.size());
			data.maxOffset         = static_cast<uint32_t>(submeshIndices.size() + subMeshRawIndices[i].size());
			data.materialDataIndex = m_materialNb + subMeshMaterialIndices[i];
			meshData.subMeshData.emplace_back(std::move(data));

			submeshIndices.insert(submeshIndices.end(), subMeshRawIndices[i].begin(), subMeshRawIndices[i].end());

			meshData.subMeshesIndexBuffer = Buffer(m_device, Span<uint32_t>(submeshIndices), BufferUsage::IndexBuffer);

			IndexedUniform<BufferSpan> bufferDescriptors;
			const std::size_t          transformFrom = m_transformNumber * m_transformOffsetSize;
			bufferDescriptors[0] = m_transformBuffer.get(transformFrom, transformFrom + sizeof(Transforms));

			const auto& materials = mesh.materials;
			for (const auto& material : materials)
			{
				IndexedUniform<vzt::Texture*> texturesDescriptors;
				const std::size_t             materialFrom = m_materialNb * m_materialInfoOffsetSize;
				bufferDescriptors[1] = m_materialInfoBuffer.get(materialFrom, materialFrom + sizeof(GenericMaterial));

				const auto genericMaterial = GenericMaterial::fromMaterial(material);
				m_materialInfoBuffer.update(genericMaterial, materialFrom);

				if (material.texture)
				{
					meshData.textureData.emplace_back();

					auto& textureData      = meshData.textureData.back();
					textureData.imageView  = {m_device, *material.texture, Format::R8G8B8A8SRGB};
					textureData.texture    = {m_device, &meshData.textureData.back().imageView};
					texturesDescriptors[2] = &textureData.texture;
				}

				m_descriptorPool.allocate(m_imageCount, m_meshDescriptorLayout);
				for (std::size_t i = 0; i < m_imageCount; i++)
					m_descriptorPool.update(m_materialNb * m_imageCount + i, bufferDescriptors, texturesDescriptors);

				m_materialNb++;
			}
		}

		m_transformNumber++;
	}

	void MeshView::configure(const vzt::Device* device, uint32_t imageCount)
	{
		m_scene->forAll<MeshDeviceData>([&](Entity entity) { entity.remove<MeshDeviceData>(); });

		m_imageCount      = imageCount;
		m_device          = device;
		m_materialNb      = 0;
		m_transformNumber = 0;
		m_meshDescriptorLayout.configure(device);

		m_descriptorPool = vzt::DescriptorPool(
		    m_device, {vzt::DescriptorType::UniformBuffer, vzt::DescriptorType::CombinedSampler}, 512);

		m_transformOffsetSize = static_cast<uint32_t>(m_device->computeUniformOffsetAlignment<Transforms>());
		m_transformBuffer     = Buffer(m_device, std::vector<uint8_t>(m_maxSupportedMesh * m_transformOffsetSize),
		                               BufferUsage::UniformBuffer, MemoryUsage::PreferDevice, true);

		m_materialInfoOffsetSize = static_cast<uint32_t>(m_device->computeUniformOffsetAlignment<Material>());
		m_materialInfoBuffer     = Buffer(m_device, std::vector<uint8_t>(m_maxSupportedMesh * m_materialInfoOffsetSize),
		                                  BufferUsage::UniformBuffer, MemoryUsage::PreferDevice, true);

		m_scene->forAll<Mesh>([&](Entity entity) { add(*entity.registry(), entity.entity()); });
		m_connection = m_scene->onConstruct<Mesh, &MeshView::add>(*this);
	}

	void MeshView::record(uint32_t imageCount, VkCommandBuffer commandBuffer, GraphicPipeline* pipeline) const
	{
		m_scene->forAll<Mesh>([&](Entity entity) {
			const auto&  deviceData      = entity.get<MeshDeviceData>();
			VkBuffer     vertexBuffers[] = {deviceData.vertexBuffer.vkHandle()};
			VkDeviceSize offsets[]       = {0};

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			const auto& indexBuffer = deviceData.subMeshesIndexBuffer;
			const auto& submeshes   = deviceData.subMeshData;
			for (const auto& subMesh : submeshes)
			{
				vkCmdBindIndexBuffer(commandBuffer, indexBuffer.vkHandle(), subMesh.minOffset * sizeof(uint32_t),
				                     VK_INDEX_TYPE_UINT32);

				m_descriptorPool.bind((subMesh.materialDataIndex) * m_imageCount + imageCount, commandBuffer,
				                      VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout());

				vkCmdDrawIndexed(commandBuffer, subMesh.maxOffset - subMesh.minOffset, 1, 0, 0, 0);
			}
		});
	}

	void MeshView::update(const Entity& cameraEntity)
	{
		const auto& [setup, view]   = cameraEntity.get<Camera, Transform>();
		const Mat4 viewMatrix       = setup.getViewMatrix(view);
		Mat4       projectionMatrix = setup.getProjectionMatrix();
		projectionMatrix[1][1] *= -1;

		std::size_t currentMaterialIndex = 0;
		m_scene->forAll<Mesh>([&](Entity entity) {
			const auto& deviceData = entity.get<MeshDeviceData>();

			const Transform* transform   = entity.try_get<Transform>();
			const Mat4       modelMatrix = transform ? transform->get() : Mat4(1.f);

			const Mat4 modelViewMatrix = viewMatrix * modelMatrix;

			const Transforms transforms = {modelViewMatrix, projectionMatrix,
			                               glm::transpose(glm::inverse(modelViewMatrix))};
			m_transformBuffer.update(transforms, deviceData.transformIndex * m_transformOffsetSize);

			const Mesh& mesh      = entity.get<Mesh>();
			const auto& materials = mesh.materials;
			for (const auto& material : materials)
			{
				const auto genericMaterial = GenericMaterial::fromMaterial(material);
				m_materialInfoBuffer.update(genericMaterial, currentMaterialIndex * m_materialInfoOffsetSize);
				currentMaterialIndex++;
			}
		});
	}
} // namespace vzt
