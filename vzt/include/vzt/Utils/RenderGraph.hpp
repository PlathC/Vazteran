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
#include "vzt/Vulkan/Texture.hpp"

namespace vzt
{
    enum class QueueType : uint8_t;
    class Swapchain;

    struct AttachmentBuilder
    {
        View<Device>       device;
        ImageUsage         usage;
        Optional<Format>   format{};    // if unset, use swapchain image
        Optional<Extent2D> imageSize{}; // if unset, use swapchain image size
        SampleCount        sampleCount = vzt::SampleCount::Sample1;
    };

    struct StorageBuilder
    {
        View<Device> device;
        std::size_t  size;
        BufferUsage  usage;
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
        bool operator<(const Handle& other) const { return id < other.id; }
    };

    template <class Type>
    using HandleMap = std::unordered_map<Handle, Type, Handle::hash>;

    class RenderGraph;
    class Pass
    {
      public:
        Pass(const Pass&)            = delete;
        Pass& operator=(const Pass&) = delete;

        Pass(Pass&&) noexcept   = default;
        Pass& operator=(Pass&&) = default;

        ~Pass() = default;

        void addColorInput(uint32_t binding, const Handle& attachment, std::string name = "");
        void addColorOutput(Handle& attachment, std::string attachmentName = "");
        void addColorInputOutput(Handle& attachment, std::string inName = "", std::string outName = "");

        void addStorageInput(uint32_t binding, const Handle& storage, std::string storageName = "",
                             Optional<Range<std::size_t>> range = {});
        void addStorageOutput(Handle& storage, std::string storageName = "", Optional<Range<std::size_t>> range = {});
        void addStorageInputOutput(Handle& storage, std::string inName = "", std::string outName = "",
                                   Optional<Range<std::size_t>> range = {});

        void setDepthInput(const Handle& depthStencil, std::string attachmentName = "");
        void setDepthOutput(Handle& depthStencil, std::string attachmentName = "");

        bool isDependingOn(const Pass& other) const;

        inline View<Queue>             getQueue() const;
        inline void                    setDescriptorLayout(DescriptorLayout&& layout);
        inline const DescriptorLayout& getDescriptorLayout() const;
        inline DescriptorLayout&       getDescriptorLayout();

        friend RenderGraph;

      private:
        Pass(std::string name, View<Queue> queue);

        std::string      m_name;
        View<Queue>      m_queue;
        std::size_t      m_id;
        DescriptorLayout m_descriptorLayout;

        struct PassAttachment
        {
            Handle        handle;
            std::string   name;
            AttachmentUse use;
            uint32_t      binding = ~0u;

            bool operator<(const PassAttachment& other) const { return handle.id < other.handle.id; }
        };

        struct PassStorage
        {
            Handle                       handle;
            std::string                  name;
            Optional<Range<std::size_t>> range;
            uint32_t                     binding = ~0u;

            bool operator<(const PassStorage& other) const { return handle.id < other.handle.id; }
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
        RenderGraph(View<Swapchain> swapchain);

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
        void create();

        static inline std::atomic<std::size_t> m_handleCounter = 0;

        View<Swapchain> m_swapchain;

        HandleMap<AttachmentBuilder> m_attachmentBuilders;
        HandleMap<StorageBuilder>    m_storageBuilders;

        std::hash<std::size_t>   m_hash{};
        std::vector<std::size_t> m_executionOrder;
        HandleMap<std::size_t>   m_handleToPhysical;
        std::vector<Pass>        m_passes;

        std::vector<RenderPass>     m_renderPasses;    // [#pass]
        std::vector<DescriptorPool> m_descriptorPools; // [#pass]
        std::vector<FrameBuffer>    m_frameBuffers;    // [passId    * #swapchainImage + swapchainImageId]
        std::vector<Image>          m_images;          // [imageId   * #swapchainImage + swapchainImageId]
        std::vector<Buffer>         m_storages;        // [storageId * #swapchainImage + swapchainImageId]

        std::vector<Texture> m_textureSaves;

        Optional<Handle> m_backBuffer;
    };
} // namespace vzt

#include "vzt/Utils/RenderGraph.inl"

#endif // VZT_UTILS_RENDERGRAPH_HPP
