#ifndef VZT_UTILS_RENDERGRAPH_HPP
#define VZT_UTILS_RENDERGRAPH_HPP

#include "vzt/Vulkan/Attachment.hpp"
#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/GraphicPipeline.hpp"

namespace vzt
{
    struct AttachmentBuilder
    {
        ImageUsage         usage;
        Optional<Format>   format{};    // if unset, use swapchain image formats
        Optional<Extent2D> imageSize{}; // if unset, use frame buffer size
        SampleCount        sampleCount = vzt::SampleCount::Sample1;
    };

    struct StorageBuilder
    {
        std::size_t size;
        BufferUsage usage;
    };

} // namespace vzt

#endif // VZT_UTILS_RENDERGRAPH_HPP
