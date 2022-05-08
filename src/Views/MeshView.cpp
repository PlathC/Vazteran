#include "Vazteran/Views/MeshView.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"

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

	MeshView::MeshView() {}

	MeshView::~MeshView() = default;

	void MeshView::addModel(const vzt::Model* const model)
	{
		ModelDisplayInformation modelDisplayInfo = {model};

		// Use one vertex buffer for the whole mesh
		const auto& vertices = model->cMesh().cVertices();
		const auto& normals  = model->cMesh().cNormals();
		const auto& uvs      = model->cMesh().cUvs();

		std::vector<vzt::TriangleVertexInput> verticesData;
		verticesData.reserve(vertices.size());
		for (std::size_t i = 0; i < vertices.size(); i++)
		{
			verticesData.emplace_back(vzt::TriangleVertexInput{vertices[i], uvs[i], normals[i]});
		}

		modelDisplayInfo.vertexBuffer = vzt::Buffer(m_device, verticesData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

		auto subMeshRawIndices      = model->cMesh().getVertexIndices();
		auto subMeshMaterialIndices = model->cMesh().getMaterialIndices();

		modelDisplayInfo.subMeshData.reserve(subMeshRawIndices.size());
		std::vector<uint32_t> submeshIndices;
		submeshIndices.reserve(subMeshRawIndices.size());
		for (std::size_t i = 0; i < subMeshRawIndices.size(); i++)
		{
			modelDisplayInfo.subMeshData.emplace_back(
			    SubMeshData{static_cast<uint32_t>(submeshIndices.size()),
			                static_cast<uint32_t>(submeshIndices.size() + subMeshRawIndices[i].size()),
			                m_materialNb + subMeshMaterialIndices[i]});

			submeshIndices.insert(submeshIndices.end(), subMeshRawIndices[i].begin(), subMeshRawIndices[i].end());
		}

		modelDisplayInfo.subMeshesIndexBuffer =
		    vzt::Buffer(m_device, sizeof(uint32_t) * submeshIndices.size(),
		                reinterpret_cast<uint8_t*>(submeshIndices.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		IndexedUniform<vzt::BufferDescriptor> bufferDescriptors;
		bufferDescriptors[0] = vzt::BufferDescriptor{static_cast<uint32_t>(m_models.size() * m_transformOffsetSize),
		                                             sizeof(vzt::Transforms), &m_transformBuffer};

		const auto& materials = model->cMesh().cMaterials();
		for (const auto& material : materials)
		{
			IndexedUniform<vzt::Texture*> texturesDescriptors;
			bufferDescriptors[1] = vzt::BufferDescriptor{m_materialNb * m_materialInfoOffsetSize,
			                                             sizeof(vzt::GenericMaterial), &m_materialInfoBuffer};
			auto genericMaterial = vzt::GenericMaterial::fromMaterial(material);

			m_materialInfoBuffer.update(sizeof(vzt::GenericMaterial), m_materialNb * m_materialInfoOffsetSize,
			                            reinterpret_cast<uint8_t*>(&genericMaterial));

			if (material.texture.has_value())
			{
				auto textureData = TextureData{
				    std::make_unique<vzt::ImageView>(m_device, material.texture.value(), vzt::Format::R8G8B8A8SRGB),
				};

				textureData.texture    = std::make_unique<vzt::Texture>(m_device, textureData.imageView.get());
				texturesDescriptors[2] = textureData.texture.get();

				modelDisplayInfo.textureData.emplace_back(std::move(textureData));
			}

			m_descriptorPool.allocate(m_imageCount, m_meshDescriptorLayout);
			for (std::size_t i = 0; i < m_imageCount; i++)
			{
				m_descriptorPool.update(m_materialNb * m_imageCount + i, bufferDescriptors, texturesDescriptors);
			}
			m_materialNb++;
		}

		m_models.emplace_back(std::move(modelDisplayInfo));
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

		m_transformOffsetSize = sizeof(vzt::Transforms);
		if (minOffset > 0)
		{
			m_transformOffsetSize = (m_transformOffsetSize + minOffset - 1) & ~(minOffset - 1);
		}

		m_transformBuffer = vzt::Buffer(m_device, std::vector<uint8_t>(128 * m_transformOffsetSize),
		                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vzt::MemoryUsage::CPU_TO_GPU);

		m_materialInfoOffsetSize = sizeof(vzt::Transforms);
		if (minOffset > 0)
		{
			m_materialInfoOffsetSize = (m_materialInfoOffsetSize + minOffset - 1) & ~(minOffset - 1);
		}

		m_materialInfoBuffer = vzt::Buffer(m_device, std::vector<uint8_t>(128 * m_materialInfoOffsetSize),
		                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vzt::MemoryUsage::CPU_TO_GPU);
	}

	void MeshView::record(uint32_t imageCount, VkCommandBuffer commandBuffer, GraphicPipeline* pipeline) const
	{
		for (const auto& model : m_models)
		{
			VkBuffer     vertexBuffers[] = {model.vertexBuffer.vkHandle()};
			VkDeviceSize offsets[]       = {0};

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			const auto& indexBuffer = model.subMeshesIndexBuffer;
			const auto& submeshes   = model.subMeshData;
			for (const auto& subMesh : submeshes)
			{
				vkCmdBindIndexBuffer(commandBuffer, indexBuffer.vkHandle(), subMesh.minOffset * sizeof(uint32_t),
				                     VK_INDEX_TYPE_UINT32);

				m_descriptorPool.bind((subMesh.materialDataIndex) * m_imageCount + imageCount, commandBuffer,
				                      VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout());

				vkCmdDrawIndexed(commandBuffer, subMesh.maxOffset - subMesh.minOffset, 1, 0, 0, 0);
			}
		}
	}

	void MeshView::update(const vzt::Camera& camera)
	{
		const auto viewMatrix       = camera.getViewMatrix();
		auto       projectionMatrix = camera.getProjectionMatrix();
		projectionMatrix[1][1] *= -1;

		std::size_t currentMaterialIndex = 0;
		for (std::size_t i = 0; i < m_models.size(); i++)
		{
			const auto&     modelDisplayInfo = m_models[i];
			const vzt::Mat4 modelMatrix      = modelDisplayInfo.modelData->getModelMatrix();

			const vzt::Mat4 modelViewMatrix = viewMatrix * modelMatrix;

			const vzt::Transforms transforms =
			    vzt::Transforms{modelViewMatrix, projectionMatrix, glm::transpose(glm::inverse(modelViewMatrix))};

			m_transformBuffer.update(sizeof(vzt::Transforms), i * m_transformOffsetSize,
			                         reinterpret_cast<const uint8_t* const>(&transforms));

			const auto& materials = modelDisplayInfo.modelData->cMesh().cMaterials();
			for (const auto& material : materials)
			{
				const auto genericMaterial = vzt::GenericMaterial::fromMaterial(material);
				m_materialInfoBuffer.update(sizeof(vzt::GenericMaterial),
				                            currentMaterialIndex * m_materialInfoOffsetSize,
				                            reinterpret_cast<const uint8_t*>(&genericMaterial));
				currentMaterialIndex++;
			}
		}
	}
} // namespace vzt
