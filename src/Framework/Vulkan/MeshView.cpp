#include "Vazteran/Framework/Vulkan/MeshView.hpp"
#include "Vazteran/Framework/Vulkan/Device.hpp"
#include "Vazteran/Framework/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"

namespace vzt
{
	VkVertexInputBindingDescription BlinnPhongVertexInput::GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding   = 0;
		bindingDescription.stride    = sizeof(vzt::BlinnPhongVertexInput);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> BlinnPhongVertexInput::GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
		    std::vector<VkVertexInputAttributeDescription>(3);

		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset   = offsetof(vzt::BlinnPhongVertexInput, position);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset   = offsetof(vzt::BlinnPhongVertexInput, textureCoordinates);

		attributeDescriptions[2].binding  = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset   = offsetof(vzt::BlinnPhongVertexInput, normal);

		return attributeDescriptions;
	}

	MeshView::MeshView(vzt::Device* device, uint32_t imageCount) : m_imageCount(imageCount), m_device(device)
	{
		vzt::Program blinnPhongProgram = vzt::Program(device);

		auto vtxShader  = vzt::Shader("./shaders/blinn-phong.vert.spv", ShaderStage::VertexShader);
		auto fragShader = vzt::Shader("./shaders/blinn-phong.frag.spv", ShaderStage::FragmentShader);

		// vtxShader.(sizeof(vzt::Transforms));
		vtxShader.SetUniformDescriptorSet(0, sizeof(vzt::Transforms));
		m_transformDescriptor = vtxShader.UniformDescriptorSets()[0];
		blinnPhongProgram.SetShader(std::move(vtxShader));

		fragShader.SetUniformDescriptorSet(1, sizeof(vzt::MaterialInfo));
		fragShader.SetSamplerDescriptorSet(2);
		fragShader.SetSamplerDescriptorSet(3);
		fragShader.SetSamplerDescriptorSet(4);
		m_materialDescriptors = fragShader.UniformDescriptorSets()[0];
		m_samplersDescriptors = fragShader.SamplerDescriptorSets();

		blinnPhongProgram.SetShader(std::move(fragShader));

		blinnPhongProgram.Compile();

		m_graphicPipeline = std::make_unique<vzt::GraphicPipeline>(
		    m_device, std::move(blinnPhongProgram),
		    vzt::VertexInputDescription{BlinnPhongVertexInput::GetBindingDescription(),
		                                BlinnPhongVertexInput::GetAttributeDescription()});

		const uint64_t minOffset = m_device->MinUniformOffsetAligment();

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

	MeshView::~MeshView() = default;

	void MeshView::AddModel(const vzt::Model* const model)
	{
		// m_models.emplace_back(model);
		ModelDisplayInformation modelDisplayInfo = {model};

		const auto& program = m_graphicPipeline->Program();

		// Use one vertex buffer for the whole mesh
		const auto& vertices = model->CMesh().CVertices();
		const auto& normals  = model->CMesh().CNormals();
		const auto& uvs      = model->CMesh().CUvs();

		std::vector<vzt::BlinnPhongVertexInput> verticesData;
		verticesData.reserve(vertices.size());
		for (std::size_t i = 0; i < vertices.size(); i++)
		{
			verticesData.emplace_back(vzt::BlinnPhongVertexInput{vertices[i], uvs[i], normals[i]});
		}

		modelDisplayInfo.vertexBuffer = vzt::Buffer(m_device, verticesData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

		auto subMeshRawIndices      = model->CMesh().VertexIndices();
		auto subMeshMaterialIndices = model->CMesh().MaterialIndices();

		modelDisplayInfo.subMeshData.reserve(subMeshRawIndices.size());
		std::vector<uint32_t> submeshIndices;
		submeshIndices.reserve(subMeshRawIndices.size());
		for (std::size_t i = 0; i < subMeshRawIndices.size(); i++)
		{
			modelDisplayInfo.subMeshData.emplace_back(SubMeshData{
			    static_cast<uint32_t>(submeshIndices.size()),
			    static_cast<uint32_t>(submeshIndices.size() + subMeshRawIndices[i].size()), subMeshMaterialIndices[i]});

			submeshIndices.insert(submeshIndices.end(), subMeshRawIndices[i].begin(), subMeshRawIndices[i].end());
		}
		modelDisplayInfo.subMeshesIndexBuffer =
		    vzt::Buffer(m_device, sizeof(uint32_t) * submeshIndices.size(),
		                reinterpret_cast<uint8_t*>(submeshIndices.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		IndexedUniform<vzt::BufferDescriptor> bufferDescriptors;
		bufferDescriptors[m_transformDescriptor.binding] = {
		    static_cast<uint32_t>(m_models.size() * m_transformOffsetSize), m_transformDescriptor.size,
		    &m_transformBuffer};

		auto materials = model->CMesh().CMaterials();
		for (std::size_t materialId = 0; materialId < materials.size(); materialId++)
		{
			MaterialData materialData{};
			materialData.descriptorPool = vzt::DescriptorPool(m_device, program.DescriptorTypes(), 8);

			materialData.textures.emplace_back(std::pair<vzt::ImageView, vzt::Sampler>{
			    vzt::ImageView(m_device, materials[materialId].diffuseMap, VK_FORMAT_R8G8B8A8_SRGB),
			    vzt::Sampler(m_device)});
			materialData.textures.emplace_back(std::pair<vzt::ImageView, vzt::Sampler>{
			    vzt::ImageView(m_device, materials[materialId].ambientMap, VK_FORMAT_R8G8B8A8_SRGB),
			    vzt::Sampler(m_device)});
			materialData.textures.emplace_back(std::pair<vzt::ImageView, vzt::Sampler>{
			    vzt::ImageView(m_device, materials[materialId].specularMap, VK_FORMAT_R8G8B8A8_SRGB),
			    vzt::Sampler(m_device)});

			IndexedUniform<vzt::ImageDescriptor> texturesDescriptors;
			texturesDescriptors[m_samplersDescriptors[0].binding] = {&materialData.textures[0].first,
			                                                         &materialData.textures[0].second};
			texturesDescriptors[m_samplersDescriptors[1].binding] = {&materialData.textures[1].first,
			                                                         &materialData.textures[1].second};
			texturesDescriptors[m_samplersDescriptors[2].binding] = {&materialData.textures[2].first,
			                                                         &materialData.textures[2].second};

			bufferDescriptors[m_materialDescriptors.binding] =
			    BufferDescriptor{m_currentMaterialInfoOffset, m_materialDescriptors.size, &m_materialInfoBuffer};

			auto materialInfo =
			    vzt::MaterialInfo{materials[materialId].ambientColor, materials[materialId].diffuseColor,
			                      materials[materialId].specularColor, materials[materialId].shininess};

			m_materialInfoBuffer.Update(sizeof(vzt::MaterialInfo), m_currentMaterialInfoOffset,
			                            reinterpret_cast<uint8_t*>(&materialInfo));

			m_currentMaterialInfoOffset += m_materialInfoOffsetSize;

			materialData.descriptorPool.Allocate(m_imageCount, program.DescriptorSetLayout());
			materialData.descriptorPool.UpdateAll(bufferDescriptors, texturesDescriptors);

			modelDisplayInfo.materialsData.emplace_back(std::move(materialData));
		}

		m_models.emplace_back(std::move(modelDisplayInfo));
	}

	void MeshView::Configure(vzt::PipelineSettings settings) { m_graphicPipeline->Configure(settings); }

	void MeshView::Record(uint32_t imageCount, VkCommandBuffer commandBuffer, const vzt::RenderPass* const renderPass)
	{
		m_graphicPipeline->Bind(commandBuffer, renderPass);
		for (const auto& model : m_models)
		{
			VkBuffer     vertexBuffers[] = {model.vertexBuffer.VkHandle()};
			VkDeviceSize offsets[]       = {0};

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			const auto& indexBuffer = model.subMeshesIndexBuffer;
			const auto& submeshes   = model.subMeshData;
			for (const auto& subMesh : submeshes)
			{
				vkCmdBindIndexBuffer(commandBuffer, indexBuffer.VkHandle(), subMesh.minOffset * sizeof(uint32_t),
				                     VK_INDEX_TYPE_UINT32);

				model.materialsData[subMesh.materialDataIndex].descriptorPool.Bind(
				    imageCount, commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline->Layout());

				vkCmdDrawIndexed(commandBuffer, subMesh.maxOffset - subMesh.minOffset, 1, 0, 0, 0);
			}
		}
	}

	void MeshView::Update(const vzt::Camera& camera)
	{
		const auto viewMatrix       = camera.View();
		auto       projectionMatrix = camera.Projection();
		projectionMatrix[1][1] *= -1;

		for (std::size_t i = 0; i < m_models.size(); i++)
		{
			const auto&     modelDisplayInfo = m_models[i];
			const vzt::Mat4 modelMatrix      = modelDisplayInfo.modelData->ModelMatrix();

			const vzt::Mat4 modelViewMatrix = viewMatrix * modelMatrix;

			const vzt::Transforms transforms =
			    vzt::Transforms{modelViewMatrix, projectionMatrix, glm::transpose(glm::inverse(modelViewMatrix))};

			m_transformBuffer.Update(sizeof(vzt::Transforms), i * m_transformOffsetSize,
			                         reinterpret_cast<const uint8_t* const>(&transforms));
		}
	}
} // namespace vzt
