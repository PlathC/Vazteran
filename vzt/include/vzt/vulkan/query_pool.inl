#include "vzt/vulkan/query_pool.hpp"

namespace vzt
{
    template <typename ResultsType>
    bool QueryPool::getResults(uint32_t firstQuery, uint32_t queryCount, Span<ResultsType> results, std::size_t stride,
                               QueryResultFlag flags) const
    {
        const Span<uint8_t> data = {
            reinterpret_cast<uint8_t*>(results.data),
            results.size * sizeof(ResultsType),
        };

        return getResults(firstQuery, queryCount, data, stride, flags);
    }

    inline VkQueryPool QueryPool::getHandle() const { return m_handle; }
} // namespace vzt