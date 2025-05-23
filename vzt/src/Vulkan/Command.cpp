#include "vzt/Vulkan/Command.hpp"

#include <cassert>

#include "vzt/Vulkan/AccelerationStructure.hpp"
#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/FrameBuffer.hpp"
#include "vzt/Vulkan/Pipeline/ComputePipeline.hpp"
#include "vzt/Vulkan/QueryPool.hpp"
#include "vzt/Vulkan/RenderPass.hpp"

namespace vzt
{
    CommandBuffer::CommandBuffer(View<Device> device, VkCommandBuffer handle)
        : DeviceObject<VkCommandBuffer>(device, handle)
    {
    }

    void CommandBuffer::barrier(const PipelineBarrier& barrier)
    {
        std::vector<VkImageMemoryBarrier> imageBarriers{};
        imageBarriers.reserve(barrier.imageBarriers.size());

        for (const auto& baseBarrier : barrier.imageBarriers)
        {
            VkImageMemoryBarrier imageBarrier{};
            imageBarrier.sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.srcAccessMask = toVulkan(baseBarrier.src);
            imageBarrier.dstAccessMask = toVulkan(baseBarrier.dst);
            imageBarrier.oldLayout     = toVulkan(baseBarrier.oldLayout);
            imageBarrier.newLayout     = toVulkan(baseBarrier.newLayout);
            imageBarrier.srcQueueFamilyIndex =
                baseBarrier.srcQueue ? baseBarrier.srcQueue->getId() : VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.dstQueueFamilyIndex =
                baseBarrier.dstQueue ? baseBarrier.dstQueue->getId() : VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.image = baseBarrier.image->getHandle();

            imageBarrier.subresourceRange.aspectMask     = toVulkan(baseBarrier.aspect);
            imageBarrier.subresourceRange.baseMipLevel   = baseBarrier.baseLevel;
            imageBarrier.subresourceRange.levelCount     = baseBarrier.levelCount;
            imageBarrier.subresourceRange.baseArrayLayer = 0;
            imageBarrier.subresourceRange.layerCount     = 1;

            imageBarriers.emplace_back(std::move(imageBarrier));
        }

        std::vector<VkBufferMemoryBarrier> bufferBarriers{};
        bufferBarriers.reserve(barrier.bufferBarriers.size());
        for (const auto& baseBarrier : barrier.bufferBarriers)
        {
            VkBufferMemoryBarrier bufferBarrier{};
            bufferBarrier.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.srcAccessMask = toVulkan(baseBarrier.src);
            bufferBarrier.dstAccessMask = toVulkan(baseBarrier.dst);
            bufferBarrier.srcQueueFamilyIndex =
                baseBarrier.srcQueue ? baseBarrier.srcQueue->getId() : VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex =
                baseBarrier.dstQueue ? baseBarrier.dstQueue->getId() : VK_QUEUE_FAMILY_IGNORED;

            bufferBarrier.buffer = baseBarrier.buffer.buffer->getHandle();
            bufferBarrier.size   = baseBarrier.buffer.size;
            bufferBarrier.offset = baseBarrier.buffer.offset;

            bufferBarriers.emplace_back(std::move(bufferBarrier));
        }

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdPipelineBarrier(m_handle, toVulkan(barrier.src), toVulkan(barrier.dst), toVulkan(barrier.dependency),
                                   0, nullptr, static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
                                   static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());
    }

    void CommandBuffer::barrier(PipelineStage src, PipelineStage dst, ImageBarrier imageBarrier)
    {
        PipelineBarrier pipelineBarrier{src, dst, std::vector{std::move(imageBarrier)}};
        barrier(pipelineBarrier);
    }

    void CommandBuffer::barrier(PipelineStage src, PipelineStage dst, BufferBarrier bufferBarrier)
    {
        PipelineBarrier pipelineBarrier{src, dst, {}, std::vector{std::move(bufferBarrier)}};
        barrier(pipelineBarrier);
    }

    void CommandBuffer::clear(View<DeviceImage> image, ImageLayout layout, Vec4 clearColor)
    {
        VkImageSubresourceRange subresource;
        subresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource.baseMipLevel   = 0;
        subresource.levelCount     = 1;
        subresource.baseArrayLayer = 0;
        subresource.layerCount     = 1;

        VkClearColorValue      clearColorValue{clearColor.x, clearColor.y, clearColor.z, clearColor.w};
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdClearColorImage(m_handle, image->getHandle(), toVulkan(layout), &clearColorValue, 1, &subresource);
    }

    void CommandBuffer::blit(View<DeviceImage> src, ImageLayout srcLayout, View<DeviceImage> dst, ImageLayout dstLayout,
                             Filter filter, const Blit& blit)
    {
        VkBlitImageInfo2 blitInfo{};
        blitInfo.sType          = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
        blitInfo.srcImage       = src->getHandle();
        blitInfo.srcImageLayout = toVulkan(srcLayout);
        blitInfo.dstImage       = dst->getHandle();
        blitInfo.dstImageLayout = toVulkan(dstLayout);
        blitInfo.filter         = toVulkan(filter);

        VkImageBlit2 blit2{};
        blit2.sType                         = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        blit2.srcOffsets[0].x               = static_cast<int32_t>(blit.srcOffsets[0].width);
        blit2.srcOffsets[0].y               = static_cast<int32_t>(blit.srcOffsets[0].height);
        blit2.srcOffsets[0].z               = static_cast<int32_t>(blit.srcOffsets[0].depth);
        blit2.srcOffsets[1].x               = static_cast<int32_t>(blit.srcOffsets[1].width);
        blit2.srcOffsets[1].y               = static_cast<int32_t>(blit.srcOffsets[1].height);
        blit2.srcOffsets[1].z               = static_cast<int32_t>(blit.srcOffsets[1].depth);
        blit2.srcSubresource.aspectMask     = toVulkan(blit.srcAspect);
        blit2.srcSubresource.mipLevel       = blit.srcMipLevel;
        blit2.srcSubresource.baseArrayLayer = 0;
        blit2.srcSubresource.layerCount     = 1;

        blit2.dstOffsets[0].x               = static_cast<int32_t>(blit.dstOffsets[0].width);
        blit2.dstOffsets[0].y               = static_cast<int32_t>(blit.dstOffsets[0].height);
        blit2.dstOffsets[0].z               = static_cast<int32_t>(blit.dstOffsets[0].depth);
        blit2.dstOffsets[1].x               = static_cast<int32_t>(blit.dstOffsets[1].width);
        blit2.dstOffsets[1].y               = static_cast<int32_t>(blit.dstOffsets[1].height);
        blit2.dstOffsets[1].z               = static_cast<int32_t>(blit.dstOffsets[1].depth);
        blit2.dstSubresource.aspectMask     = toVulkan(blit.dstAspect);
        blit2.dstSubresource.mipLevel       = blit.dstMipLevel;
        blit2.dstSubresource.baseArrayLayer = 0;
        blit2.dstSubresource.layerCount     = 1;

        blitInfo.pRegions    = &blit2;
        blitInfo.regionCount = 1;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBlitImage2(m_handle, &blitInfo);
    }

    void CommandBuffer::blit(View<DeviceImage> src, ImageLayout srcLayout, ImageAspect srcAspect, Vec2u srcStart,
                             Vec2u srcEnd, View<DeviceImage> dst, ImageLayout dstLayout, ImageAspect dstAspect,
                             Vec2u dstStart, Vec2u dstEnd, Filter filter)
    {
        VkBlitImageInfo2 blitInfo{};
        blitInfo.sType          = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
        blitInfo.srcImage       = src->getHandle();
        blitInfo.srcImageLayout = toVulkan(srcLayout);
        blitInfo.dstImage       = dst->getHandle();
        blitInfo.dstImageLayout = toVulkan(dstLayout);
        blitInfo.filter         = toVulkan(filter);

        // The Vulkan spec states: If srcImage is of type VK_IMAGE_TYPE_1D or VK_IMAGE_TYPE_2D, then for each
        // element of pRegions, srcOffsets[0].z must be 0 and srcOffsets[1].z must be 1
        VkImageBlit2 blit{};
        blit.sType                         = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        blit.srcOffsets[0].x               = static_cast<int32_t>(srcStart.x);
        blit.srcOffsets[0].y               = static_cast<int32_t>(srcStart.y);
        blit.srcOffsets[0].z               = 0;
        blit.srcOffsets[1].x               = static_cast<int32_t>(srcEnd.x);
        blit.srcOffsets[1].y               = static_cast<int32_t>(srcEnd.y);
        blit.srcOffsets[1].z               = 1;
        blit.srcSubresource.aspectMask     = toVulkan(srcAspect);
        blit.srcSubresource.mipLevel       = 0;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount     = 1;

        blit.dstOffsets[0].x               = static_cast<int32_t>(dstStart.x);
        blit.dstOffsets[0].y               = static_cast<int32_t>(dstStart.y);
        blit.dstOffsets[0].z               = 0;
        blit.dstOffsets[1].x               = static_cast<int32_t>(dstEnd.x);
        blit.dstOffsets[1].y               = static_cast<int32_t>(dstEnd.y);
        blit.dstOffsets[1].z               = 1;
        blit.dstSubresource.aspectMask     = toVulkan(dstAspect);
        blit.dstSubresource.mipLevel       = 0;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount     = 1;

        blitInfo.pRegions    = &blit;
        blitInfo.regionCount = 1;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBlitImage2(m_handle, &blitInfo);
    }

    void CommandBuffer::blit(View<DeviceImage> src, ImageLayout srcLayout, ImageAspect srcAspect, View<DeviceImage> dst,
                             ImageLayout dstLayout, ImageAspect dstAspect, Filter filter)
    {
        const Extent3D srcExtent = src->getSize();
        blit(src, srcLayout, srcAspect, Vec2u{0}, Vec2u{srcExtent.width, srcExtent.height}, //
             dst, dstLayout, dstAspect, Vec2u{0}, Vec2u{srcExtent.width, srcExtent.height}, filter);
    }

    void CommandBuffer::blit(View<DeviceImage> src, ImageLayout srcLayout, View<DeviceImage> dst, ImageLayout dstLayout,
                             Filter filter)
    {
        const Extent3D srcExtent = src->getSize();
        blit(src, srcLayout, ImageAspect::Color, Vec2u{0}, Vec2u{srcExtent.width, srcExtent.height}, //
             dst, dstLayout, ImageAspect::Color, Vec2u{0}, Vec2u{srcExtent.width, srcExtent.height}, filter);
    }

    void CommandBuffer::copy(View<Buffer> src, View<Buffer> dst, uint64_t size, uint64_t srcOffset, uint64_t dstOffset)
    {
        VkBufferCopy copyRegion;
        copyRegion.size      = size;
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdCopyBuffer(m_handle, src->getHandle(), dst->getHandle(), 1, &copyRegion);
    }

    void CommandBuffer::copy(View<Buffer> src, View<DeviceImage> dst, uint32_t width, uint32_t height,
                             ImageAspect aspect)
    {
        VkBufferImageCopy region           = {};
        region.imageSubresource.aspectMask = vzt::toVulkan(aspect);
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width           = width;
        region.imageExtent.height          = height;
        region.imageExtent.depth           = 1;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdCopyBufferToImage(m_handle, src->getHandle(), dst->getHandle(),
                                     vzt::toVulkan(ImageLayout::TransferDstOptimal), 1, &region);
    }

    void CommandBuffer::copy(View<DeviceImage> src, View<DeviceImage> dst, uint32_t width, uint32_t height,
                             ImageAspect aspect)
    {
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = toVulkan(aspect);
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = toVulkan(aspect);
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width              = width;
        imageCopyRegion.extent.height             = height;
        imageCopyRegion.extent.depth              = 1;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdCopyImage(m_handle, src->getHandle(), vzt::toVulkan(ImageLayout::TransferSrcOptimal),
                             dst->getHandle(), vzt::toVulkan(ImageLayout::TransferDstOptimal), 1, &imageCopyRegion);
    }

    void CommandBuffer::bind(const GraphicPipeline& graphicPipeline)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline.getHandle());
    }

    void CommandBuffer::bind(const GraphicPipeline& graphicPipeline, const DescriptorSet& set)
    {
        bind(graphicPipeline);
        const VkDescriptorSet descriptorSet = set.getHandle();

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline.getLayout(), 0, 1,
                                      &descriptorSet, 0, nullptr);
    }

    void CommandBuffer::bind(const ComputePipeline& computePipeline)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.getHandle());
    }

    void CommandBuffer::bind(const ComputePipeline& computePipeline, const DescriptorSet& set)
    {
        bind(computePipeline);
        const VkDescriptorSet descriptorSet = set.getHandle();

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.getLayout(), 0, 1,
                                      &descriptorSet, 0, nullptr);
    }

    void CommandBuffer::bind(const RaytracingPipeline& raytracingPipeline)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracingPipeline.getHandle());
    }

    void CommandBuffer::bind(const RaytracingPipeline& raytracingPipeline, const DescriptorSet& set)
    {
        bind(raytracingPipeline);

        const VkDescriptorSet  descriptorSet = set.getHandle();
        const VolkDeviceTable& table         = m_device->getFunctionTable();
        table.vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracingPipeline.getLayout(),
                                      0, 1, &descriptorSet, 0, nullptr);
    }

    void CommandBuffer::bindVertexBuffer(const Buffer& buffer)
    {
        VkBuffer     vertexBuffers[] = {buffer.getHandle()};
        VkDeviceSize offsets[]       = {0};

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBindVertexBuffers(m_handle, 0, 1, vertexBuffers, offsets);
    }

    void CommandBuffer::bindIndexBuffer(const Buffer& buffer, std::size_t index)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBindIndexBuffer(m_handle, buffer.getHandle(), index * sizeof(uint32_t), VK_INDEX_TYPE_UINT32);
    }

    void CommandBuffer::dispatch(uint32_t x, uint32_t y, uint32_t z)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdDispatch(m_handle, x, y, z);
    }

    void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset,
                             uint32_t instanceOffset)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdDraw(m_handle, vertexCount, instanceCount, vertexOffset, instanceOffset);
    }

    void CommandBuffer::drawIndexed(const Buffer& indexBuffer, const Range<>& range, uint32_t instanceCount,
                                    int32_t vertexOffset, uint32_t instanceOffset)
    {
        bindIndexBuffer(indexBuffer, range.start);

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdDrawIndexed(m_handle, static_cast<uint32_t>(range.size()), instanceCount, 0, vertexOffset,
                               instanceOffset);
    }

    void CommandBuffer::drawIndirect(const BufferCSpan& buffer, uint32_t drawCount, uint32_t stride)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdDrawIndirect(m_handle, buffer.buffer->getHandle(), buffer.offset, drawCount, stride);
    }

    void CommandBuffer::drawIndexedIndirect(const BufferCSpan& buffer, uint32_t drawCount, uint32_t stride)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdDrawIndexedIndirect(m_handle, buffer.buffer->getHandle(), buffer.offset, drawCount, stride);
    }

    void CommandBuffer::dispatchIndirect(const BufferCSpan& buffer)
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdDispatchIndirect(m_handle, buffer.buffer->getHandle(), buffer.offset);
    }

    void CommandBuffer::traceRays(StridedSpan<uint64_t> raygen, StridedSpan<uint64_t> miss, StridedSpan<uint64_t> hit,
                                  StridedSpan<uint64_t> callable, uint32_t width, uint32_t height, uint32_t depth)
    {
        VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
        raygenShaderSbtEntry.deviceAddress = raygen.data;
        raygenShaderSbtEntry.stride        = raygen.stride;
        raygenShaderSbtEntry.size          = raygen.size;

        VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
        missShaderSbtEntry.deviceAddress = miss.data;
        missShaderSbtEntry.stride        = miss.stride;
        missShaderSbtEntry.size          = miss.size;

        VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
        hitShaderSbtEntry.deviceAddress = hit.data;
        hitShaderSbtEntry.stride        = hit.stride;
        hitShaderSbtEntry.size          = hit.size;

        VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};
        callableShaderSbtEntry.deviceAddress = callable.data;
        callableShaderSbtEntry.stride        = callable.stride;
        callableShaderSbtEntry.size          = callable.size;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdTraceRaysKHR(m_handle, &raygenShaderSbtEntry, &missShaderSbtEntry, &hitShaderSbtEntry,
                                &callableShaderSbtEntry, width, height, depth);
    }

    void CommandBuffer::setViewport(const Extent2D& size, float minDepth, float maxDepth)
    {
        VkViewport viewport;
        viewport.x        = 0.f;
        viewport.y        = 0.f;
        viewport.width    = static_cast<float>(size.width);
        viewport.height   = static_cast<float>(size.height);
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdSetViewport(m_handle, 0, 1, &viewport);
    }

    void CommandBuffer::setScissor(const Extent2D& size, Vec2i offset)
    {
        VkRect2D scissor;
        scissor.extent = {size.width, size.height};
        scissor.offset = VkOffset2D{offset.x, offset.y};

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdSetScissor(m_handle, 0, 1, &scissor);
    }

    void CommandBuffer::reset(const QueryPool& pool, uint32_t firstQuery, uint32_t queryCount)
    {
        vkCmdResetQueryPool(m_handle, pool.getHandle(), firstQuery, queryCount);
    }

    void CommandBuffer::writeTimeStamp(const QueryPool& pool, uint32_t query, PipelineStage waitingStage)
    {
        vkCmdWriteTimestamp(m_handle, toVulkan(waitingStage), pool.getHandle(), query);
    }

    void CommandBuffer::beginPass(const RenderPass& pass, const FrameBuffer& frameBuffer)
    {
        VkRenderPassBeginInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};

        renderPassInfo.renderPass        = pass.getHandle();
        renderPassInfo.framebuffer       = frameBuffer.getHandle();
        renderPassInfo.renderArea.offset = {0, 0};

        const auto size                  = frameBuffer.getSize();
        renderPassInfo.renderArea.extent = VkExtent2D{size.width, size.height};

        const auto& colorAttachments = pass.getColorAttachments();
        const auto& depth            = pass.getDepthAttachment();

        std::vector<VkClearValue> clearColors{};
        clearColors.reserve(colorAttachments.size() + 1);
        for (auto& attachment : colorAttachments)
        {
            const VkClearValue color = {attachment.clearValue.r, attachment.clearValue.g, attachment.clearValue.b,
                                        attachment.clearValue.a};
            clearColors.emplace_back(color);
        }

        const VkClearValue depthClear = {depth.clearValue.r, depth.clearValue.g, depth.clearValue.b,
                                         depth.clearValue.a};
        clearColors.emplace_back(depthClear);

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
        renderPassInfo.pClearValues    = clearColors.data();

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBeginRenderPass(m_handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::endPass()
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdEndRenderPass(m_handle);
    }

    void CommandBuffer::buildAs(AccelerationStructureBuilder& builder)
    {
        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationBuildGeometryInfo.type  = toVulkan(builder.as->getType());
        accelerationBuildGeometryInfo.flags = toVulkan(builder.flags);
        accelerationBuildGeometryInfo.mode  = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

        std::vector<VkAccelerationStructureGeometryKHR> vkGeometries;

        const std::vector<GeometryAccelerationStructureBuilder>& geometries = builder.as->getGeometries();
        vkGeometries.reserve(geometries.size());
        uint32_t maxPrimitiveCount = 0;
        for (std::size_t i = 0; i < geometries.size(); i++)
        {
            const auto& geometry = geometries[i];
            std::visit(Overloaded{
                           [&maxPrimitiveCount](const AccelerationStructureTriangles& trianglesAs) {
                               maxPrimitiveCount += static_cast<uint32_t>(trianglesAs.indexBuffer.buffer->size() /
                                                                          (3 * sizeof(uint32_t)));
                           },
                           [&maxPrimitiveCount](const AccelerationStructureAabbs& aabbsAs) {
                               maxPrimitiveCount +=
                                   static_cast<uint32_t>(aabbsAs.aabbs.buffer->size() / aabbsAs.stride);
                           },
                           [&maxPrimitiveCount](const AccelerationStructureInstance& instancesAs) {
                               maxPrimitiveCount += instancesAs.count;
                           },
                       },
                       geometry.geometry);

            auto vkGeometry = toVulkan(geometry);
            vkGeometries.emplace_back(std::move(vkGeometry));
        }

        accelerationBuildGeometryInfo.dstAccelerationStructure  = builder.as->getHandle();
        accelerationBuildGeometryInfo.geometryCount             = static_cast<uint32_t>(vkGeometries.size());
        accelerationBuildGeometryInfo.pGeometries               = vkGeometries.data();
        accelerationBuildGeometryInfo.scratchData.deviceAddress = vzt::align( //
            builder.scratchBuffer->getDeviceAddress(), builder.scratchBufferMinAlignment);

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount  = maxPrimitiveCount;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex     = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;

        std::vector accelerationBuildStructureRangeInfos = {&accelerationStructureBuildRangeInfo};

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkCmdBuildAccelerationStructuresKHR(m_handle, 1, &accelerationBuildGeometryInfo,
                                                  accelerationBuildStructureRangeInfos.data());
    }

    void CommandBuffer::begin()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkBeginCommandBuffer(m_handle, &beginInfo),
                "Failed to start the recording of the command buffer");
    }

    void CommandBuffer::end()
    {
        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkEndCommandBuffer(m_handle), "Failed to end command buffer recording");
    }

    CommandPool::CommandPool(View<Device> device, View<Queue> queue, uint32_t bufferNb)
        : DeviceObject<VkCommandPool>(device)
    {
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = queue->getId();
        commandPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreateCommandPool(m_device->getHandle(), &commandPoolInfo, nullptr, &m_handle),
                "Failed to create command pool.");

        allocateCommandBuffers(bufferNb);
    }

    CommandPool::CommandPool(CommandPool&& other) noexcept : DeviceObject<VkCommandPool>(std::move(other))
    {
        std::swap(m_commandBuffers, other.m_commandBuffers);
        std::swap(m_queue, other.m_queue);
    }

    CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
    {
        std::swap(m_commandBuffers, other.m_commandBuffers);
        std::swap(m_queue, other.m_queue);

        DeviceObject<VkCommandPool>::operator=(std::move(other));
        return *this;
    }

    CommandPool::~CommandPool()
    {
        if (m_handle == VK_NULL_HANDLE || m_commandBuffers.empty())
            return;

        // Avoid deleting command buffers while they're being processed by the device;
        m_device->wait();

        const uint32_t commandBufferNb = static_cast<uint32_t>(m_commandBuffers.size());

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkFreeCommandBuffers(m_device->getHandle(), m_handle, commandBufferNb, m_commandBuffers.data());
        table.vkDestroyCommandPool(m_device->getHandle(), m_handle, nullptr);
    }

    void CommandPool::allocateCommandBuffers(const uint32_t count)
    {
        if (!m_commandBuffers.empty())
        {
            const uint32_t commandBufferNb = static_cast<uint32_t>(m_commandBuffers.size());

            const VolkDeviceTable& table = m_device->getFunctionTable();
            table.vkFreeCommandBuffers(m_device->getHandle(), m_handle, commandBufferNb, m_commandBuffers.data());
            m_commandBuffers.clear();
        }

        m_commandBuffers.resize(count);

        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool        = m_handle;
        commandBufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount = count;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkAllocateCommandBuffers(m_device->getHandle(), &commandBufferAllocInfo, m_commandBuffers.data()),
                "Failed to allocate command buffers");
    }

    CommandBuffer CommandPool::operator[](uint32_t bufferNumber)
    {
        assert(bufferNumber < m_commandBuffers.size() && "bufferNumber should be < than m_commandBuffers.size()");
        return CommandBuffer(m_device, m_commandBuffers[bufferNumber]);
    }
} // namespace vzt
