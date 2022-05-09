#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"

namespace vzt
{
	DescriptorLayout::DescriptorLayout() = default;

	DescriptorLayout::DescriptorLayout(const DescriptorLayout& other)
	    : m_device(other.m_device), m_bindings(other.m_bindings)
	{
		if (m_handle != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(m_device->vkHandle(), m_handle, nullptr);
			m_handle = VK_NULL_HANDLE;
		}
	}

	DescriptorLayout& DescriptorLayout::operator=(const DescriptorLayout& other)
	{
		if (m_handle != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(m_device->vkHandle(), m_handle, nullptr);
			m_handle = VK_NULL_HANDLE;
		}
		m_device   = other.m_device;
		m_bindings = other.m_bindings;

		return *this;
	}

	DescriptorLayout::DescriptorLayout(DescriptorLayout&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_handle, other.m_handle);
		std::swap(m_bindings, other.m_bindings);
	}

	DescriptorLayout& DescriptorLayout::operator=(DescriptorLayout&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_handle, other.m_handle);
		std::swap(m_bindings, other.m_bindings);

		return *this;
	}

	void DescriptorLayout::addBinding(const vzt::ShaderStage bindingStage, const uint32_t binding,
	                                  const vzt::DescriptorType type)
	{
		m_bindings.emplace_back(Binding{binding, bindingStage, type});
	}

	void DescriptorLayout::configure(const vzt::Device* const device) { m_device = device; }

	const VkDescriptorSetLayout& DescriptorLayout::vkHandle() const
	{
		if (!m_device)
		{
			throw std::runtime_error("Descriptor set has not been configured");
		}

		if (m_handle == VK_NULL_HANDLE)
		{
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			layoutBindings.reserve(m_bindings.size());
			for (const auto& descriptor : m_bindings)
			{
				VkDescriptorSetLayoutBinding layoutBinding{};
				layoutBinding.binding            = std::get<0>(descriptor);
				layoutBinding.descriptorCount    = 1;
				layoutBinding.pImmutableSamplers = nullptr; // Optional
				layoutBinding.stageFlags         = static_cast<VkShaderStageFlags>(std::get<1>(descriptor));
				layoutBinding.descriptorType     = static_cast<VkDescriptorType>(std::get<2>(descriptor));
				layoutBindings.emplace_back(layoutBinding);
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
			layoutInfo.pBindings    = layoutBindings.data();

			if (vkCreateDescriptorSetLayout(m_device->vkHandle(), &layoutInfo, nullptr, &m_handle) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create descriptor set layout!");
			}
		}

		return m_handle;
	}

	DescriptorLayout::~DescriptorLayout()
	{
		if (m_handle != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(m_device->vkHandle(), m_handle, nullptr);
			m_handle = VK_NULL_HANDLE;
		}
	}

	DescriptorPool::DescriptorPool(const vzt::Device* const device, const std::vector<DescriptorType> descriptorTypes,
	                               const uint32_t maxSetNb, const VkDescriptorPoolCreateFlags flags)
	    : m_device(device), m_maxSetNb(maxSetNb)
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(descriptorTypes.size());
		for (const auto& descriptorType : descriptorTypes)
		{
			sizes.emplace_back(VkDescriptorPoolSize{vzt::toVulkan(descriptorType), maxSetNb});
		}

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags                      = flags;
		pool_info.maxSets                    = maxSetNb;
		pool_info.poolSizeCount              = static_cast<uint32_t>(sizes.size());
		pool_info.pPoolSizes                 = sizes.data();

		if (vkCreateDescriptorPool(m_device->vkHandle(), &pool_info, nullptr, &m_vkHandle) != VK_SUCCESS)
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

	void DescriptorPool::allocate(uint32_t count, const vzt::DescriptorLayout& layout)
	{
		const auto layouts = std::vector<VkDescriptorSetLayout>(count, layout.vkHandle());

		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool     = m_vkHandle;
		descriptorSetAllocInfo.descriptorSetCount = count;
		descriptorSetAllocInfo.pSetLayouts        = layouts.data();

		auto descriptorSets = std::vector<VkDescriptorSet>(count);
		if (vkAllocateDescriptorSets(m_device->vkHandle(), &descriptorSetAllocInfo, descriptorSets.data()) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		m_descriptors.insert(m_descriptors.end(), descriptorSets.begin(), descriptorSets.end());
	}

	void DescriptorPool::bind(uint32_t i, VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
	                          VkPipelineLayout pipelineLayout) const
	{
		vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, pipelineLayout, 0, 1, &m_descriptors[i], 0, nullptr);
	}

	void DescriptorPool::updateAll(const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors,
	                               const IndexedUniform<vzt::Texture*>&         textureHandlers)
	{
		for (std::size_t i = 0; i < m_descriptors.size(); i++)
		{
			update(i, bufferDescriptors, textureHandlers);
		}
	}

	void DescriptorPool::updateAll(const IndexedUniform<vzt::Texture*>& imageDescriptors)
	{
		for (std::size_t i = 0; i < m_descriptors.size(); i++)
		{
			update(i, imageDescriptors);
		}
	}

	void DescriptorPool::updateAll(const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors)
	{
		for (std::size_t i = 0; i < m_descriptors.size(); i++)
		{
			update(i, bufferDescriptors);
		}
	}

	void DescriptorPool::update(const std::size_t i, const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors,
	                            const IndexedUniform<vzt::Texture*>& imageDescriptors)
	{
		assert(i < m_descriptors.size() && "i must be less than Size()");

		auto descriptorWrites     = std::vector<VkWriteDescriptorSet>();
		auto descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(bufferDescriptors.size());

		std::size_t bufferIdx = 0;
		for (const auto& bufferDescriptor : bufferDescriptors)
		{
			descriptorBufferInfo[bufferIdx].buffer = bufferDescriptor.second.buffer->vkHandle();
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
			descriptorImageInfo[bufferIdx].imageView   = imageDescriptor.second->getView()->vkHandle();
			descriptorImageInfo[bufferIdx].sampler     = imageDescriptor.second->getSampler()->vkHandle();

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

		vkUpdateDescriptorSets(m_device->vkHandle(), static_cast<uint32_t>(descriptorWrites.size()),
		                       descriptorWrites.data(), 0, nullptr);
	}

	void DescriptorPool::update(std::size_t i, const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors)
	{
		assert(i < m_descriptors.size() && "i must be less than Size()");

		auto        descriptorWrites     = std::vector<VkWriteDescriptorSet>();
		auto        descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(bufferDescriptors.size());
		std::size_t bufferIdx            = 0;
		for (const auto& bufferDescriptor : bufferDescriptors)
		{
			descriptorBufferInfo[bufferIdx].buffer = bufferDescriptor.second.buffer->vkHandle();
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

		vkUpdateDescriptorSets(m_device->vkHandle(), static_cast<uint32_t>(descriptorWrites.size()),
		                       descriptorWrites.data(), 0, nullptr);
	}

	void DescriptorPool::update(const std::size_t i, const IndexedUniform<vzt::Texture*>& imageDescriptors)
	{
		assert(i < m_descriptors.size() && "i must be less than Size()");

		std::size_t bufferIdx           = 0;
		auto        descriptorWrites    = std::vector<VkWriteDescriptorSet>();
		auto        descriptorImageInfo = std::vector<VkDescriptorImageInfo>(imageDescriptors.size());
		for (const auto& imageDescriptor : imageDescriptors)
		{
			descriptorImageInfo[bufferIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptorImageInfo[bufferIdx].imageView   = imageDescriptor.second->getView()->vkHandle();
			descriptorImageInfo[bufferIdx].sampler     = imageDescriptor.second->getSampler()->vkHandle();

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

		vkUpdateDescriptorSets(m_device->vkHandle(), static_cast<uint32_t>(descriptorWrites.size()),
		                       descriptorWrites.data(), 0, nullptr);
	}

	DescriptorPool::~DescriptorPool()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(m_device->vkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

} // namespace vzt
