#include "vzt/Vulkan/Descriptor.hpp"

#include <cassert>

#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/Image.hpp"
#include "vzt/Vulkan/Texture.hpp"

namespace vzt
{
    DescriptorLayout::DescriptorLayout(View<Device> device) : m_device(device) {}

    DescriptorLayout::DescriptorLayout(const DescriptorLayout& other)
        : m_device(other.m_device), m_bindings(other.m_bindings)
    {
        if (other.m_handle == VK_NULL_HANDLE)
            return;

        compile();
    }

    DescriptorLayout& DescriptorLayout::operator=(const DescriptorLayout& other)
    {
        m_device   = other.m_device;
        m_bindings = other.m_bindings;

        if (other.m_handle != VK_NULL_HANDLE)
            compile();

        return *this;
    }

    DescriptorLayout::DescriptorLayout(DescriptorLayout&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_bindings, other.m_bindings);
        std::swap(m_compiled, other.m_compiled);
    }

    DescriptorLayout& DescriptorLayout::operator=(DescriptorLayout&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_bindings, other.m_bindings);
        std::swap(m_compiled, other.m_compiled);

        return *this;
    }

    DescriptorLayout::~DescriptorLayout()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(m_device->getHandle(), m_handle, nullptr);
    }

    void DescriptorLayout::addBinding(uint32_t binding, DescriptorType type)
    {
        m_bindings.emplace(binding, type);
        m_compiled = false;
    }

    void DescriptorLayout::compile()
    {
        if (m_compiled)
            return;

        if (m_handle != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(m_device->getHandle(), m_handle, nullptr);

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(m_bindings.size());
        for (const auto& [binding, type] : m_bindings)
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding         = binding;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType  = toVulkan(type);
            layoutBinding.stageFlags      = VK_SHADER_STAGE_ALL;
            layoutBindings.emplace_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings    = layoutBindings.data();

        vkCheck(vkCreateDescriptorSetLayout(m_device->getHandle(), &layoutInfo, nullptr, &m_handle),
                "Failed to create descriptor set layout!");

        m_compiled = true;
    }

    DescriptorPool::DescriptorPool(View<Device> device, std::unordered_set<DescriptorType> descriptorTypes,
                                   uint32_t maxSetNb)
        : m_device(device), m_maxSetNb(maxSetNb)
    {
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(descriptorTypes.size());
        for (const auto& descriptorType : descriptorTypes)
            sizes.emplace_back(VkDescriptorPoolSize{toVulkan(descriptorType), maxSetNb});

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags                      = 0;
        pool_info.maxSets                    = maxSetNb;
        pool_info.poolSizeCount              = static_cast<uint32_t>(sizes.size());
        pool_info.pPoolSizes                 = sizes.data();

        vkCheck(vkCreateDescriptorPool(m_device->getHandle(), &pool_info, nullptr, &m_handle),
                "Failed to create descriptor pool.");
    }

    DescriptorPool::DescriptorPool(View<Device> device, const DescriptorLayout& descriptorLayout, uint32_t maxSetNb)
        : m_device(device), m_maxSetNb(maxSetNb)
    {
        const auto& bindings = descriptorLayout.getBindings();

        std::unordered_set<DescriptorType> types{};
        types.reserve(bindings.size());
        for (const auto& [_, descriptorType] : bindings)
            types.emplace(descriptorType);

        std::vector<VkDescriptorPoolSize> sizes{};
        sizes.reserve(types.size());
        for (const auto& descriptorType : types)
            sizes.emplace_back(VkDescriptorPoolSize{toVulkan(descriptorType), maxSetNb});

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags                      = 0;
        pool_info.maxSets                    = maxSetNb;
        pool_info.poolSizeCount              = static_cast<uint32_t>(sizes.size());
        pool_info.pPoolSizes                 = sizes.data();

        vkCheck(vkCreateDescriptorPool(m_device->getHandle(), &pool_info, nullptr, &m_handle),
                "Failed to create descriptor pool.");
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_descriptors, other.m_descriptors);
        std::swap(m_maxSetNb, other.m_maxSetNb);
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_descriptors, other.m_descriptors);
        std::swap(m_maxSetNb, other.m_maxSetNb);

        return *this;
    }

    DescriptorPool::~DescriptorPool()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(m_device->getHandle(), m_handle, nullptr);
    }

    void DescriptorPool::allocate(uint32_t count, const DescriptorLayout& layout)
    {
        const auto layouts = std::vector(count, layout.getHandle());

        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool     = m_handle;
        descriptorSetAllocInfo.descriptorSetCount = count;
        descriptorSetAllocInfo.pSetLayouts        = layouts.data();

        std::vector<VkDescriptorSet> descriptorSets{count};
        vkCheck(vkAllocateDescriptorSets(m_device->getHandle(), &descriptorSetAllocInfo, descriptorSets.data()),
                "Failed to allocate descriptor sets.");

        m_descriptors.insert(m_descriptors.end(), descriptorSets.begin(), descriptorSets.end());
    }

    void DescriptorPool::update(const Indexed<BufferSpan>&    bufferDescriptors,
                                const Indexed<View<Texture>>& imageDescriptors)
    {
        for (std::size_t i = 0; i < m_descriptors.size(); i++)
            update(i, bufferDescriptors, imageDescriptors);
    }

    void DescriptorPool::update(const Indexed<View<Texture>>& imageDescriptors)
    {
        for (std::size_t i = 0; i < m_descriptors.size(); i++)
            update(i, imageDescriptors);
    }

    void DescriptorPool::update(const Indexed<BufferSpan>& bufferDescriptors)
    {
        for (std::size_t i = 0; i < m_descriptors.size(); i++)
            update(i, bufferDescriptors);
    }

    void DescriptorPool::update(const std::size_t descriptorId, const Indexed<BufferSpan>& bufferDescriptors,
                                const Indexed<View<Texture>>& imageDescriptors)
    {
        assert(descriptorId < m_descriptors.size() && "i must be less than Size()");

        std::vector<VkWriteDescriptorSet>   descriptorWrites{};
        std::vector<VkDescriptorBufferInfo> descriptorBufferInfo{bufferDescriptors.size()};

        std::size_t bufferIdx = 0;
        for (const auto& [binding, descriptor] : bufferDescriptors)
        {
            descriptorBufferInfo[bufferIdx].buffer = descriptor.data->getHandle();
            descriptorBufferInfo[bufferIdx].offset = descriptor.offset;
            descriptorBufferInfo[bufferIdx].range  = descriptor.size;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = m_descriptors[descriptorId];
            descriptorWrite.dstBinding      = binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo     = &descriptorBufferInfo[bufferIdx];
            descriptorWrites.emplace_back(descriptorWrite);
            bufferIdx++;
        }

        bufferIdx = 0;
        std::vector<VkDescriptorImageInfo> descriptorImageInfo{imageDescriptors.size()};
        for (const auto& [binding, texture] : imageDescriptors)
        {
            descriptorImageInfo[bufferIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo[bufferIdx].imageView   = texture->getView()->getHandle();
            descriptorImageInfo[bufferIdx].sampler     = texture->getSampler().getHandle();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = m_descriptors[descriptorId];
            descriptorWrite.dstBinding      = binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo      = &descriptorImageInfo[bufferIdx];
            descriptorWrites.emplace_back(descriptorWrite);
            bufferIdx++;
        }

        vkUpdateDescriptorSets(m_device->getHandle(), static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(), 0, nullptr);
    }

    void DescriptorPool::update(std::size_t descriptorId, const Indexed<BufferSpan>& bufferDescriptors)
    {
        assert(descriptorId < m_descriptors.size() && "descriptorId must be less than size()");

        std::vector<VkWriteDescriptorSet>   descriptorWrites{};
        std::vector<VkDescriptorBufferInfo> descriptorBufferInfo{bufferDescriptors.size()};
        std::size_t                         bufferIdx = 0;
        for (const auto& [binding, buffer] : bufferDescriptors)
        {
            descriptorBufferInfo[bufferIdx].buffer = buffer.data->getHandle();
            descriptorBufferInfo[bufferIdx].offset = buffer.offset;
            descriptorBufferInfo[bufferIdx].range  = buffer.size;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = m_descriptors[descriptorId];
            descriptorWrite.dstBinding      = binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo     = &descriptorBufferInfo[bufferIdx];
            descriptorWrites.emplace_back(descriptorWrite);
            bufferIdx++;
        }

        vkUpdateDescriptorSets(m_device->getHandle(), static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(), 0, nullptr);
    }

    void DescriptorPool::update(const std::size_t descriptorId, const Indexed<View<Texture>>& imageDescriptors)
    {
        assert(descriptorId < m_descriptors.size() && "descriptorId must be less than size()");

        std::size_t                        bufferIdx = 0;
        std::vector<VkWriteDescriptorSet>  descriptorWrites{};
        std::vector<VkDescriptorImageInfo> descriptorImageInfo{imageDescriptors.size()};
        for (const auto& [binding, view] : imageDescriptors)
        {
            descriptorImageInfo[bufferIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo[bufferIdx].imageView   = view->getView()->getHandle();
            descriptorImageInfo[bufferIdx].sampler     = view->getSampler().getHandle();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = m_descriptors[descriptorId];
            descriptorWrite.dstBinding      = binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo      = &descriptorImageInfo[bufferIdx];
            descriptorWrites.emplace_back(descriptorWrite);
            bufferIdx++;
        }

        vkUpdateDescriptorSets(m_device->getHandle(), static_cast<uint32_t>(descriptorWrites.size()),
                               descriptorWrites.data(), 0, nullptr);
    }

} // namespace vzt