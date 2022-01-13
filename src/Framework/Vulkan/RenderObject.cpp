#include "Vazteran/Framework/Vulkan/RenderObject.hpp"
#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"

namespace vzt
{
	RenderObject::RenderObject(
	    vzt::Device *device, vzt::GraphicPipeline *graphicPipeline, vzt::Model *model, uint32_t imageCount)
	    : m_imageCount(imageCount), m_device(device), m_model(model)
	{

		// Create descriptor pools
		auto descriptorTypes = graphicPipeline->DescriptorTypes();
		auto poolSizes = std::vector<VkDescriptorPoolSize>(descriptorTypes.size());

		for (std::size_t i = 0; i < descriptorTypes.size(); i++)
		{
			poolSizes[i].type = descriptorTypes[i];
			poolSizes[i].descriptorCount = static_cast<uint32_t>(m_imageCount);
		}

		// Use one vertex buffer for the whole mesh
		m_vertexBuffer = std::make_unique<vzt::VertexBuffer>(
		    m_device, model->CMesh().CVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

		// Handle submesh indices
		auto subMeshRawIndices = model->CMesh().VertexIndices();
		auto subMeshMaterialIndices = model->CMesh().MaterialIndices();
		std::vector<uint32_t> indices;
		indices.reserve(subMeshRawIndices.size());
		for (std::size_t i = 0; i < subMeshRawIndices.size(); i++)
		{
			m_subMeshData.emplace_back(SubMeshData{
			    subMeshMaterialIndices[i], static_cast<uint32_t>(indices.size()),
			    static_cast<uint32_t>(indices.size() + subMeshRawIndices[i].size())});
			indices.insert(indices.end(), subMeshRawIndices[i].begin(), subMeshRawIndices[i].end());
		}
		m_subMeshesIndexBuffer =
		    std::make_unique<vzt::IndexBuffer>(m_device, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		auto materials = model->CMesh().CMaterials();
		m_descriptorPools.resize(materials.size());
		for (auto &shader : graphicPipeline->Shaders())
		{
			auto uniformDescriptorSets = shader.UniformDescriptorSets();
			for (auto &uniform : uniformDescriptorSets)
			{
				m_uniformRanges[uniform.binding] = uniform.size;
			}

			auto samplerDescriptorSets = shader.SamplerDescriptorSets();
			if (samplerDescriptorSets.size() == 3)
			{
				for (std::size_t materialId = 0; materialId < materials.size(); materialId++)
				{
					VkDescriptorPoolCreateInfo descriptorPoolInfo{};
					descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
					descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
					descriptorPoolInfo.pPoolSizes = poolSizes.data();
					descriptorPoolInfo.maxSets = static_cast<uint32_t>(m_imageCount);

					if (vkCreateDescriptorPool(
					        m_device->VkHandle(), &descriptorPoolInfo, nullptr, &m_descriptorPools[materialId]) !=
					    VK_SUCCESS)
					{
						throw std::runtime_error("Failed to create descriptor pool!");
					}

					auto layouts =
					    std::vector<VkDescriptorSetLayout>(m_imageCount, graphicPipeline->DescriptorSetLayout());
					VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
					descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
					descriptorSetAllocInfo.descriptorPool = m_descriptorPools[materialId];
					descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(m_imageCount);
					descriptorSetAllocInfo.pSetLayouts = layouts.data();

					std::unordered_map<uint32_t, vzt::ImageHandler> textureHandlers;
					vzt::ImageHandler ambientImage = {
					    vzt::ImageView(m_device, materials[materialId].ambientMap, VK_FORMAT_R8G8B8A8_SRGB),
					    vzt::Sampler(m_device)};
					textureHandlers.insert({samplerDescriptorSets[0].binding, std::move(ambientImage)});
					vzt::ImageHandler diffuseImage = {
					    vzt::ImageView(m_device, materials[materialId].diffuseMap, VK_FORMAT_R8G8B8A8_SRGB),
					    vzt::Sampler(m_device)};
					textureHandlers.insert({samplerDescriptorSets[1].binding, std::move(diffuseImage)});
					vzt::ImageHandler specularImage = {
					    vzt::ImageView(m_device, materials[materialId].specularMap, VK_FORMAT_R8G8B8A8_SRGB),
					    vzt::Sampler(m_device)};
					textureHandlers.insert({samplerDescriptorSets[2].binding, std::move(specularImage)});

					auto descriptorSets = std::vector<VkDescriptorSet>(m_imageCount);
					if (vkAllocateDescriptorSets(device->VkHandle(), &descriptorSetAllocInfo, descriptorSets.data()) !=
					    VK_SUCCESS)
					{
						throw std::runtime_error("Failed to allocate descriptor sets!");
					}

					auto uniformBuffers = std::vector<vzt::Buffer<vzt::MaterialInfo>>();
					uniformBuffers.reserve(m_imageCount);
					auto materialInfo = vzt::MaterialInfo{
					    materials[materialId].ambientColor, materials[materialId].diffuseColor,
					    materials[materialId].specularColor, materials[materialId].shininess};
					for (std::size_t i = 0; i < descriptorSets.size(); i++)
					{
						uniformBuffers.emplace_back(vzt::Buffer<vzt::MaterialInfo>(
						    m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

						UpdateDescriptorSet(descriptorSets[i], uniformBuffers[i].VkHandle(), textureHandlers);

						vkDeviceWaitIdle(m_device->VkHandle());
						uniformBuffers[i].Update<vzt::MaterialInfo>({materialInfo});
					}

					m_materialData.emplace_back(std::make_unique<MaterialData>(
					    MaterialData{std::move(textureHandlers), descriptorSets, std::move(uniformBuffers)}));
				}
			}
		}
	}

	void RenderObject::Render(
	    VkCommandBuffer commandBuffer, const vzt::GraphicPipeline *graphicPipeline, uint32_t imageCount)
	{
		VkBuffer vertexBuffers[] = {m_vertexBuffer->VkHandle()};
		VkDeviceSize offsets[] = {0};

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		for (const auto &subMesh : m_subMeshData)
		{
			vkCmdBindIndexBuffer(
			    commandBuffer, m_subMeshesIndexBuffer->VkHandle(), subMesh.minOffset * sizeof(uint32_t),
			    VK_INDEX_TYPE_UINT32);

			const auto &currentDescriptorSets = m_materialData[subMesh.materialDataIndex]->descriptorSets;
			vkCmdBindDescriptorSets(
			    commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline->Layout(), 0, 1,
			    &currentDescriptorSets[imageCount], 0, nullptr);

			vkCmdPushConstants(
			    commandBuffer, graphicPipeline->Layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vzt::Transforms),
			    &m_currentPushConstants);

			vkCmdDrawIndexed(commandBuffer, subMesh.maxOffset - subMesh.minOffset, 1, 0, 0, 0);
		}
	}

	void RenderObject::UpdateDescriptorSet(
	    VkDescriptorSet descriptorSet, VkBuffer uniformBuffer,
	    const std::unordered_map<uint32_t, vzt::ImageHandler> &textureHandlers)
	{
		auto descriptorWrites = std::vector<VkWriteDescriptorSet>();
		auto descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(m_uniformRanges.size());
		std::size_t i = 0;
		for (const auto &uniformRanges : m_uniformRanges)
		{
			descriptorBufferInfo[i].buffer = uniformBuffer;
			descriptorBufferInfo[i].offset = 0;
			descriptorBufferInfo[i].range = uniformRanges.second;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = uniformRanges.first;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &descriptorBufferInfo[i];
			descriptorWrites.emplace_back(descriptorWrite);
			i++;
		}

		i = 0;
		auto descriptorImageInfo = std::vector<VkDescriptorImageInfo>(textureHandlers.size());
		for (const auto &textureHandler : textureHandlers)
		{
			descriptorImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptorImageInfo[i].imageView = textureHandler.second.imageView.VkHandle();
			descriptorImageInfo[i].sampler = textureHandler.second.sampler.VkHandle();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = textureHandler.first;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &descriptorImageInfo[i];
			descriptorWrites.emplace_back(descriptorWrite);
			i++;
		}

		vkUpdateDescriptorSets(
		    m_device->VkHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void RenderObject::UpdatePushConstants(const vzt::Transforms &objectData)
	{
		m_currentPushConstants = objectData;
	}

	RenderObject::~RenderObject()
	{
		for (const auto &descriptorPool : m_descriptorPools)
		{
			vkDestroyDescriptorPool(m_device->VkHandle(), descriptorPool, nullptr);
		}
	};
} // namespace vzt
