#include "vzt/vulkan/pipeline/pipeline.hpp"

namespace vzt
{
    Pipeline::~Pipeline() { destroy(); }

    void Pipeline::createLayout()
    {
        std::vector<VkPushConstantRange> pushConstants{};
        for (const auto [stages, offset, size] : m_pushConstants)
            pushConstants.push_back({static_cast<VkShaderStageFlags>(stages), offset, size});

        const VkDescriptorSetLayout descriptorSetLayout = m_descriptorLayout.getHandle();

        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount         = 1;
        createInfo.pSetLayouts            = &descriptorSetLayout;
        createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        createInfo.pPushConstantRanges    = pushConstants.data();

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreatePipelineLayout(m_device->getHandle(), &createInfo, nullptr, &m_pipelineLayout),
                "Failed to create pipeline layout.");
    }

    void Pipeline::destroy()
    {
        if (!m_device)
            return;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkDestroyPipeline(m_device->getHandle(), m_handle, nullptr);
        table.vkDestroyPipelineLayout(m_device->getHandle(), m_pipelineLayout, nullptr);

        m_handle         = VK_NULL_HANDLE;
        m_pipelineLayout = VK_NULL_HANDLE;
    }
} // namespace vzt
