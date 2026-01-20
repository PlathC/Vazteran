#ifndef VZT_VULKAN_QUERYPOOL_HPP
#define VZT_VULKAN_QUERYPOOL_HPP

#include "vzt/core/type.hpp"
#include "vzt/vulkan/device.hpp"

namespace vzt
{
    class Device;

    class QueryPool : public DeviceObject<VkQueryPool>
    {
      public:
        QueryPool() = default;
        QueryPool(View<Device> device, QueryType type, uint32_t count);

        QueryPool(const QueryPool& pool)            = delete;
        QueryPool& operator=(const QueryPool& pool) = delete;

        QueryPool(QueryPool&& pool) noexcept            = default;
        QueryPool& operator=(QueryPool&& pool) noexcept = default;

        ~QueryPool() override;

        template <typename ResultsType>
        void getResults(uint32_t firstQuery, uint32_t queryCount, Span<ResultsType> results, std::size_t stride,
                        QueryResultFlag flags) const;
        void getResults(uint32_t firstQuery, uint32_t queryCount, Span<uint8_t> results, std::size_t stride,
                        QueryResultFlag flags) const;
        inline VkQueryPool getHandle() const;
    };
} // namespace vzt

#include "vzt/vulkan/query_pool.inl"

#endif // VZT_VULKAN_QUERYPOOL_HPP
