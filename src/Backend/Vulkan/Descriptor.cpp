#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	DescriptorPool::DescriptorPool(Device* device, const std::vector<DescriptorType> descriptorTypes, uint32_t maxSetNb,
	                               const VkDescriptorPoolCreateFlags flags)
	    : m_device(device), m_maxSetNb(maxSetNb)
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(descriptorTypes.size());
		for (const auto& descriptorType : descriptorTypes)
		{
			sizes.emplace_back(
			    VkDescriptorPoolSize{static_cast<VkDescriptorType>(vzt::ToUnderlying(descriptorType)), maxSetNb});
		}

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags                      = flags;
		pool_info.maxSets                    = maxSetNb;
		pool_info.poolSizeCount              = (uint32_t)sizes.size();
		pool_info.pPoolSizes                 = sizes.data();

		if (vkCreateDescriptorPool(m_device->VkHandle(), &pool_info, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool!");
		}
	}

	DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_descriptors, other.m_descriptors);
		std::swap(m_maxSetNb, other.m_maxSetNb);
	}

	DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_descriptors, other.m_descriptors);
		std::swap(m_maxSetNb, other.m_maxSetNb);
		return *this;
	}

	void DescriptorPool::Allocate(uint32_t count, VkDescriptorSetLayout layout)
	{
		const auto layouts = std::vector<VkDescriptorSetLayout>(count, layout);

		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool     = m_vkHandle;
		descriptorSetAllocInfo.descriptorSetCount = count;
		descriptorSetAllocInfo.pSetLayouts        = layouts.data();

		auto descriptorSets = std::vector<VkDescriptorSet>(count);
		if (vkAllocateDescriptorSets(m_device->VkHandle(), &descriptorSetAllocInfo, descriptorSets.data()) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		m_descriptors.insert(m_descriptors.end(), descriptorSets.begin(), descriptorSets.end());
	}

	void DescriptorPool::Bind(uint32_t i, VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
	                          VkPipelineLayout pipelineLayout) const
	{
		vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, pipelineLayout, 0, 1, &m_descriptors[i], 0, nullptr);
	}

	void DescriptorPool::UpdateAll(const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors,
	                               const IndexedUniform<vzt::Texture*>&         textureHandlers)
	{
		for (std::size_t i = 0; i < m_descriptors.size(); i++)
		{
			Update(i, bufferDescriptors, textureHandlers);
		}
	}
	void DescriptorPool::UpdateAll(const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors)
	{
		for (std::size_t i = 0; i < m_descriptors.size(); i++)
		{
			Update(i, bufferDescriptors);
		}
	}

	void DescriptorPool::Update(const std::size_t i, const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors,
	                            const IndexedUniform<vzt::Texture*>& imageDescriptors)
	{
		assert(i < m_descriptors.size() && "i must be less than Size()");

		auto descriptorWrites     = std::vector<VkWriteDescriptorSet>();
		auto descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(bufferDescriptors.size());

		std::size_t bufferIdx = 0;
		for (const auto& bufferDescriptor : bufferDescriptors)
		{
			descriptorBufferInfo[bufferIdx].buffer = bufferDescriptor.second.buffer->VkHandle();
			descriptorBufferInfo[bufferIdx].offset = bufferDescriptor.second.offset;
			descriptorBufferInfo[bufferIdx].range  = bufferDescriptor.second.range;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet          = m_descriptors[i];
			descriptorWrite.dstBinding      = bufferDescriptor.first;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo     = &descriptorBufferInfo[bufferIdx];
			descriptorWrites.emplace_back(descriptorWrite);
			bufferIdx++;
		}

		bufferIdx                = 0;
		auto descriptorImageInfo = std::vector<VkDescriptorImageInfo>(imageDescriptors.size());
		for (const auto& imageDescriptor : imageDescriptors)
		{
			descriptorImageInfo[bufferIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptorImageInfo[bufferIdx].imageView   = imageDescriptor.second->View()->VkHandle();
			descriptorImageInfo[bufferIdx].sampler     = imageDescriptor.second->Sampler()->VkHandle();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet          = m_descriptors[i];
			descriptorWrite.dstBinding      = imageDescriptor.first;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo      = &descriptorImageInfo[bufferIdx];
			descriptorWrites.emplace_back(descriptorWrite);
			bufferIdx++;
		}

		vkUpdateDescriptorSets(m_device->VkHandle(), static_cast<uint32_t>(descriptorWrites.size()),
		                       descriptorWrites.data(), 0, nullptr);
	}

	void DescriptorPool::Update(std::size_t i, const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors)
	{
		assert(i < m_descriptors.size() && "i must be less than Size()");

		auto descriptorWrites     = std::vector<VkWriteDescriptorSet>();
		auto descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(bufferDescriptors.size());
		for (const auto& bufferDescriptor : bufferDescriptors)
		{
			descriptorBufferInfo[i].buffer = bufferDescriptor.second.buffer->VkHandle();
			descriptorBufferInfo[i].offset = bufferDescriptor.second.offset;
			descriptorBufferInfo[i].range  = bufferDescriptor.second.range;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet          = m_descriptors[i];
			descriptorWrite.dstBinding      = bufferDescriptor.first;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo     = &descriptorBufferInfo[i];
			descriptorWrites.emplace_back(descriptorWrite);
		}

		vkUpdateDescriptorSets(m_device->VkHandle(), static_cast<uint32_t>(descriptorWrites.size()),
		                       descriptorWrites.data(), 0, nullptr);
	}

	DescriptorPool::~DescriptorPool()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(m_device->VkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

} // namespace vzt
