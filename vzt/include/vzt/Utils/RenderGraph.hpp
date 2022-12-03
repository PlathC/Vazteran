#ifndef VZT_UTILS_RENDERGRAPH_HPP
#define VZT_UTILS_RENDERGRAPH_HPP

#include <atomic>
#include <functional>

#include "vzt/Vulkan/Attachment.hpp"
#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/Command.hpp"
#include "vzt/Vulkan/FrameBuffer.hpp"
#include "vzt/Vulkan/GraphicPipeline.hpp"

namespace vzt
{
    enum class QueueType : uint8_t;

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

    enum class HandleType
    {
        Attachment,
        Storage
    };

    struct Handle
    {
        std::size_t id;
        HandleType  type;
        std::size_t state = 0; // Allows to differentiate version of handle

        struct hash
        {
            std::size_t operator()(const Handle& handle) const { return handle.id; }
        };

        bool operator==(const Handle& other) const { return id == other.id; }
    };

    template <class Type>
    using HandleMap = std::unordered_map<Handle, Type, Handle::hash>;

    class Pass
    {
      public:
        Pass(const std::string& name, View<Queue> queue);

        bool isDependingOn(const Pass& other) const;

      private:
        std::vector<Handle> m_colorInputs;
        std::vector<Handle> m_storageInputs;
        Optional<Handle>    m_depthInput;

        std::vector<Handle> m_colorOutputs;
        std::vector<Handle> m_storageOutputs;
        Optional<Handle>    m_depthOutput;
    };

    class RenderGraph
    {
      public:
        RenderGraph() = default;

        // User configuration
        Handle addAttachment(AttachmentBuilder builder);
        Handle addStorage(StorageBuilder builder);
        Pass&  addPass(const std::string& name, View<Queue> queue);

        void setBackBuffer(const Handle handle);
        bool isBackBuffer(const Handle handle) const;

        // User information check
        void compile();
        void record();

      private:
        Handle generateAttachmentHandle() const;
        Handle generateStorageHandle() const;

        void sort();
        void reorder();

        static inline std::atomic<std::size_t> m_handleCounter = 0;

        struct PhysicalAttachment
        {
            AttachmentBuilder settings;
        };

        struct PhysicalStorage
        {
            StorageBuilder settings;
            Buffer         buffer;
        };

        HandleMap<AttachmentBuilder> m_attachmentBuilders;
        HandleMap<StorageBuilder>    m_storageBuilders;

        std::hash<std::size_t>   m_hash{};
        std::vector<std::size_t> m_executionOrder;
        std::vector<Pass>        m_passes;

        std::vector<FrameBuffer> m_frameBuffers; // [#swapchainImage * #Pass]
        std::vector<Attachment>  m_attachments;  // [#swapchainImage * #Pass]

        CommandPool      m_commandPool;
        Optional<Handle> m_backBuffer;

        Extent2D m_swapchainSize;
    };

} // namespace vzt

#endif // VZT_UTILS_RENDERGRAPH_HPP
