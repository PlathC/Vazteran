#ifndef VZT_VULKAN_QUERYPOOL_HPP
#define VZT_VULKAN_QUERYPOOL_HPP

#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Core.hpp"
#include "vzt/Vulkan/DeviceObject.hpp"

namespace vzt
{
    class Device;

    enum class QueryType : uint32_t
    {
        Occlusion          = VK_QUERY_TYPE_OCCLUSION,
        PipelineStatistics = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        Timestamp          = VK_QUERY_TYPE_TIMESTAMP,
        // Provided by VK_KHR_video_queue
        ResultStatusOnlyKHR = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR,
        // Provided by VK_EXT_transform_feedback
        TransformFeedbackStreamEXT = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT,
        // Provided by VK_KHR_performance_query
        PerformanceQueryKHR = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureCompactedSizeKHR = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureSerializationSizeKHR = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR,
        // Provided by VK_INTEL_performance_query
        PerformanceQueryIntel = VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL,
        // Provided by VK_EXT_mesh_shader
        MeshPrimitivesGeneratedEXT = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT,
        // Provided by VK_EXT_primitives_generated_query
        PrimitivesGeneratedEXT = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT,
        // Provided by VK_KHR_ray_tracing_maintenance1
        AccelerationStructureSerializationBottomLevelPointers =
            VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,
        // Provided by VK_KHR_ray_tracing_maintenance1
        AccelerationStructureSizeKHR = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,
        // Provided by VK_EXT_opacity_micromap
        MicromapSerializationSizeEXT = VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT,
        // Provided by VK_EXT_opacity_micromap
        MicromapCompactedSizeEXT = VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(QueryType, VkQueryType)

    enum class QueryResultFlag : uint32_t
    {
        N64              = VK_QUERY_RESULT_64_BIT,
        Wait             = VK_QUERY_RESULT_WAIT_BIT,
        WithAvailability = VK_QUERY_RESULT_WITH_AVAILABILITY_BIT,
        Partial          = VK_QUERY_RESULT_PARTIAL_BIT,
        // Provided by VK_KHR_video_queue
        WithStatusKHR = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(QueryResultFlag, VkQueryResultFlagBits)
    VZT_DEFINE_BITWISE_FUNCTIONS(QueryResultFlag)

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

#include "vzt/Vulkan/QueryPool.inl"

#endif // VZT_VULKAN_QUERYPOOL_HPP
