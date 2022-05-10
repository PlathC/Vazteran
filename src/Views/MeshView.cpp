#include "Vazteran/Views/MeshView.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/System/Transform.hpp"

namespace vzt
{
	VkVertexInputBindingDescription TriangleVertexInput::getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding   = 0;
		bindingDescription.stride    = sizeof(vzt::TriangleVertexInput);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> TriangleVertexInput::getAttributeDescription()
	{
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

		return attributeDescriptions;
	}

	MeshView::MeshView(Scene& scene) : m_scene(&scene) {}

	MeshView::~MeshView() {}

	void MeshView::add(Entity meshEntity)
	{
		const Mesh& mesh = meshEntity.get<Mesh>();

		const auto& vertices = mesh.vertices;
		const auto& normals  = mesh.normals;
		const auto& uvs      = mesh.uvs;

		std::vector<vzt::TriangleVertexInput> verticesData;
		verticesData.reserve(vertices.size());
		for (std::size_t i = 0; i < vertices.size(); i++)
		{
			verticesData.emplace_back(vzt::TriangleVertexInput{vertices[i], uvs[i], normals[i]});
		}

		MeshDeviceData& meshData    = meshEntity.emplace<MeshDeviceData>();
		meshData.vertexBuffer       = {m_device, verticesData, BufferUsage::VertexBuffer};
		meshData.transformIndex     = m_transformNumber;
		auto subMeshRawIndices      = mesh.getVertexIndices();
		auto subMeshMaterialIndices = mesh.getMaterialIndices();

		meshData.subMeshData.reserve(subMeshRawIndices.size());
		std::vector<uint32_t> submeshIndices;
		submeshIndices.reserve(subMeshRawIndices.size());
		for (std::size_t i = 0; i < subMeshRawIndices.size(); i++)
		{
			meshData.subMeshData.emplace_back();
			meshData.subMeshData.back().minOffset = static_cast<uint32_t>(submeshIndices.size());
			meshData.subMeshData.back().maxOffset =
			    static_cast<uint32_t>(submeshIndices.size() + subMeshRawIndices[i].size());
			meshData.subMeshData.back().materialDataIndex = m_materialNb + subMeshMaterialIndices[i];

			submeshIndices.insert(submeshIndices.end(), subMeshRawIndices[i].begin(), subMeshRawIndices[i].end());

			meshData.subMeshesIndexBuffer = {m_device, sizeof(uint32_t) * submeshIndices.size(),
			                                 reinterpret_cast<uint8_t*>(submeshIndices.data()),
			                                 BufferUsage::IndexBuffer};

			IndexedUniform<vzt::BufferDescriptor> bufferDescriptors;
			bufferDescriptors[0]        = {};
			bufferDescriptors[0].buffer = &m_transformBuffer;
			bufferDescriptors[0].offset = m_transformNumber * m_transformOffsetSize;
			bufferDescriptors[0].range  = sizeof(vzt::Transforms);

			const auto& materials = mesh.materials;
			for (const auto& material : materials)
			{
				IndexedUniform<vzt::Texture*> texturesDescriptors;
				bufferDescriptors[1]        = {};
				bufferDescriptors[1].offset = m_materialNb * m_materialInfoOffsetSize;
				bufferDescriptors[1].range  = sizeof(vzt::GenericMaterial);
				bufferDescriptors[1].buffer = &m_materialInfoBuffer;

				const auto genericMaterial = vzt::GenericMaterial::fromMaterial(material);
				m_materialInfoBuffer.update(sizeof(vzt::GenericMaterial), m_materialNb * m_materialInfoOffsetSize,
				                            reinterpret_cast<const uint8_t*>(&genericMaterial));

				if (material.texture)
				{
					meshData.textureData.emplace_back();

					auto& textureData      = meshData.textureData.back();
					textureData.imageView  = {m_device, *material.texture, vzt::Format::R8G8B8A8SRGB};
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
		m_imageCount           = imageCount;
		m_device               = device;
		m_meshDescriptorLayout = vzt::DescriptorLayout();
		m_meshDescriptorLayout.configure(device);
		m_meshDescriptorLayout.addBinding(vzt::ShaderStage::VertexShader, 0, vzt::DescriptorType::UniformBuffer);
		m_meshDescriptorLayout.addBinding(vzt::ShaderStage::FragmentShader, 1, vzt::DescriptorType::UniformBuffer);
		m_meshDescriptorLayout.addBinding(vzt::ShaderStage::FragmentShader, 2, vzt::DescriptorType::CombinedSampler);

		m_descriptorPool = vzt::DescriptorPool(
		    m_device, {vzt::DescriptorType::UniformBuffer, vzt::DescriptorType::CombinedSampler}, 512);

		const uint32_t minOffset = static_cast<uint32_t>(m_device->getMinUniformOffsetAlignment());
		m_transformOffsetSize    = sizeof(vzt::Transforms);
		if (minOffset > 0)
			m_transformOffsetSize = (m_transformOffsetSize + minOffset - 1) & ~(minOffset - 1);

		m_transformBuffer = vzt::Buffer(m_device, std::vector<uint8_t>(m_maxSupportedMesh * m_transformOffsetSize),
		                                BufferUsage::UniformBuffer, MemoryUsage::PreferDevice, true);

		m_materialInfoOffsetSize = sizeof(vzt::Transforms);
		if (minOffset > 0)
			m_materialInfoOffsetSize = (m_materialInfoOffsetSize + minOffset - 1) & ~(minOffset - 1);

		m_materialInfoBuffer =
		    vzt::Buffer(m_device, std::vector<uint8_t>(m_maxSupportedMesh * m_materialInfoOffsetSize),
		                BufferUsage::UniformBuffer, MemoryUsage::PreferDevice, true);

		m_scene->forAll<Mesh>([&](Entity entity) { add(entity); });

		m_meshListener = Listener<Mesh>(m_scene, [&](Entity entity, SystemEvent eventType) {
			if (eventType == SystemEvent::Construct)
				add(entity);
		});
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

	void MeshView::update(const vzt::Camera& camera)
	{
		const auto viewMatrix       = camera.getViewMatrix();
		auto       projectionMatrix = camera.getProjectionMatrix();
		projectionMatrix[1][1] *= -1;

		std::size_t currentMaterialIndex = 0;
		m_scene->forAll<Mesh>([&](Entity entity) {
			const auto& deviceData = entity.get<MeshDeviceData>();

			const Transform* transform   = entity.try_get<Transform>();
			const vzt::Mat4  modelMatrix = transform ? transform->get() : Mat4(1.f);

			const vzt::Mat4 modelViewMatrix = viewMatrix * modelMatrix;

			const vzt::Transforms transforms =
			    vzt::Transforms{modelViewMatrix, projectionMatrix, glm::transpose(glm::inverse(modelViewMatrix))};

			m_transformBuffer.update(sizeof(vzt::Transforms), deviceData.transformIndex * m_transformOffsetSize,
			                         reinterpret_cast<const uint8_t* const>(&transforms));

			const Mesh& mesh      = entity.get<Mesh>();
			const auto& materials = mesh.materials;
			for (const auto& material : materials)
			{
				const auto genericMaterial = vzt::GenericMaterial::fromMaterial(material);
				m_materialInfoBuffer.update(sizeof(vzt::GenericMaterial),
				                            currentMaterialIndex * m_materialInfoOffsetSize,
				                            reinterpret_cast<const uint8_t*>(&genericMaterial));
				currentMaterialIndex++;
			}
		});
	}
} // namespace vzt
