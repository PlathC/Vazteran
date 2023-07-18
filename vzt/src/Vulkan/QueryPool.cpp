#include "vzt/Vulkan/QueryPool.hpp"

#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    QueryPool::QueryPool(View<Device> device, QueryType type, uint32_t count) : m_device(device)
    {
        VkQueryPoolCreateInfo createInfo = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.queryType             = toVulkan(type);
        createInfo.queryCount            = count;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreateQueryPool(m_device->getHandle(), &createInfo, nullptr, &m_handle),
                "Failed to create VkQueryPool!");
    }

    QueryPool::QueryPool(QueryPool&& other) noexcept
        : m_device(std::exchange(other.m_device, nullptr)), m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    {
    }

    QueryPool& QueryPool::operator=(QueryPool&& other) noexcept
    {
        m_device = std::exchange(other.m_device, nullptr);
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);

        return *this;
    }

    QueryPool::~QueryPool()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkDestroyQueryPool(m_device->getHandle(), m_handle, nullptr);
    }

    void QueryPool::getResults(uint32_t firstQuery, uint32_t queryCount, Span<uint8_t> results, std::size_t stride,
                               QueryResultFlag flags) const
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkGetQueryPoolResults(m_device->getHandle(), m_handle, firstQuery, queryCount, results.size,
                                            results.data, stride, toVulkan(flags)),
                "Failed to obtain QueryPool results!");
    }
} // namespace vzt