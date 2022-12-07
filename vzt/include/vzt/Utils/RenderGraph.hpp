#ifndef VZT_UTILS_RENDERGRAPH_HPP
#define VZT_UTILS_RENDERGRAPH_HPP

#include <atomic>
#include <functional>
#include <unordered_set>

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

    class RecordHandler
    {
      public:
        virtual ~RecordHandler()                                                           = default;
        virtual void record(uint32_t i, const DescriptorSet& set, CommandBuffer& commands) = 0;
    };

    using RecordCallback = std::function<void(uint32_t i, const DescriptorSet& set, CommandBuffer& commands)>;
    class LambdaRecorder : public RecordHandler
    {
      public:
        LambdaRecorder(RecordCallback callback);
        ~LambdaRecorder() override = default;

        void record(uint32_t i, const DescriptorSet& set, CommandBuffer& commands) override;

      private:
        RecordCallback m_callback;
    };

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

        void setRecordFunction(std::unique_ptr<RecordHandler>&& recordCallback);

        bool isDependingOn(const Pass& other) const;
        void record(uint32_t i, const DescriptorSet& set, CommandBuffer& commands) const;

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

        std::unique_ptr<RecordHandler> m_recordCallback;

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

        std::vector<PassAttachment> m_colorInputs;
        std::vector<PassStorage>    m_storageInputs;
        Optional<PassAttachment>    m_depthInput;

        std::vector<PassAttachment> m_colorOutputs;
        std::vector<PassStorage>    m_storageOutputs;
        Optional<PassAttachment>    m_depthOutput;
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

        void record(uint32_t i, CommandBuffer& commands);

      private:
        Handle generateAttachmentHandle() const;
        Handle generateStorageHandle() const;

        View<Image>  getImage(uint32_t swapchainImageId, Handle handle) const;
        View<Buffer> getStorage(uint32_t swapchainImageId, Handle handle) const;

        const AttachmentBuilder& getConfiguration(Handle handle);

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

        class PhysicalPass
        {
          public:
            PhysicalPass(RenderGraph& graph, Pass& pass, Format depthFormat);

            PhysicalPass(const PhysicalPass&)            = default;
            PhysicalPass& operator=(const PhysicalPass&) = default;

            PhysicalPass(PhysicalPass&&)            = default;
            PhysicalPass& operator=(PhysicalPass&&) = default;

            ~PhysicalPass() = default;

            void record(uint32_t i, CommandBuffer& commands);

          private:
            View<RenderGraph>    m_graph;
            View<Pass>           m_pass;
            Optional<RenderPass> m_renderPass;
            DescriptorPool       m_pool;

            std::vector<FrameBuffer> m_frameBuffers; // [swapchainImageId]
            std::vector<Texture>     m_textureSaves;
        };
        friend PhysicalPass;

        std::vector<Image>        m_images;   // [imageId  ]
        std::vector<Buffer>       m_storages; // [storageId]
        std::vector<PhysicalPass> m_physicalPasses;

        Optional<Handle> m_backBuffer;
    };
} // namespace vzt

#include "vzt/Utils/RenderGraph.inl"

#endif // VZT_UTILS_RENDERGRAPH_HPP
