#include "vzt/Vulkan/QueryPool.hpp"

namespace vzt
{
    template <typename ResultsType>
    void QueryPool::getResults(uint32_t firstQuery, uint32_t queryCount, Span<ResultsType> results, std::size_t stride,
                               QueryResultFlag flags) const
    {
        const Span<uint8_t> data = {
            reinterpret_cast<uint8_t*>(results.data),
            results.size * sizeof(ResultsType),
        };

        getResults(firstQuery, queryCount, data, stride, flags);
    }
    inline VkQueryPool QueryPool::getHandle() const { return m_handle; }
} // namespace vzt