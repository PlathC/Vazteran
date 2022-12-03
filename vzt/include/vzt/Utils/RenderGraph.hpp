#ifndef VZT_UTILS_RENDERGRAPH_HPP
#define VZT_UTILS_RENDERGRAPH_HPP

#include <atomic>
#include <functional>
#include <set>

#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/Command.hpp"
#include "vzt/Vulkan/FrameBuffer.hpp"
#include "vzt/Vulkan/GraphicPipeline.hpp"
#include "vzt/Vulkan/RenderPass.hpp"

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

    class RenderGraph;
    class Pass
    {
      public:
        void addColorInput(const Handle& attachment, std::string name = "");
        void addColorOutput(Handle& attachment, std::string attachmentName = "");
        void addColorInputOutput(Handle& attachment, std::string inName = "", std::string outName = "");

        void addStorageInput(const Handle& storage, std::string storageName = "",
                             Optional<Range<std::size_t>> range = {});
        void addStorageOutput(const Handle& storage, std::string storageName = "",
                              Optional<Range<std::size_t>> range = {});
        void addStorageInputOutput(Handle& storage, std::string inName = "", std::string outName = "",
                                   Optional<Range<std::size_t>> range = {});

        void setDepthInput(const Handle& depthStencil, std::string attachmentName = "");
        void setDepthOutput(Handle& depthStencil, std::string attachmentName = "");

        bool isDependingOn(const Pass& other) const;

        friend RenderGraph;

      private:
        Pass(std::string name, View<Queue> queue);

        std::string m_name;
        View<Queue> m_queue;

        struct PassAttachment
        {
            Handle        handle;
            std::string   name;
            AttachmentUse use;

            bool operator<(const PassAttachment& other) const { return handle.id < other.handle.id; }
        };

        struct PassStorage
        {
            Handle                       handle;
            std::string                  name;
            Optional<Range<std::size_t>> range;

            bool operator<(const PassAttachment& other) const { return handle.id < other.handle.id; }
        };

        std::set<PassAttachment> m_colorInputs;
        std::set<PassStorage>    m_storageInputs;
        Optional<PassAttachment> m_depthInput;

        std::set<PassAttachment> m_colorOutputs;
        std::set<PassStorage>    m_storageOutputs;
        Optional<PassAttachment> m_depthOutput;
    };

    class RenderGraph
    {
      public:
        RenderGraph() = default;

        // User configuration
        Handle addAttachment(AttachmentBuilder builder);
        Handle addStorage(StorageBuilder builder);
        Pass&  addPass(std::string name, View<Queue> queue);

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
        std::vector<Image>       m_attachments;  // [#swapchainImage * #Pass]

        CommandPool      m_commandPool;
        Optional<Handle> m_backBuffer;

        Extent2D m_swapchainSize;
    };

} // namespace vzt

#endif // VZT_UTILS_RENDERGRAPH_HPP
