#ifndef VZT_UTILS_RENDERGRAPH_HPP
#define VZT_UTILS_RENDERGRAPH_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>

#include "vzt/vulkan/buffer.hpp"
#include "vzt/vulkan/command.hpp"
#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/frame_buffer.hpp"
#include "vzt/vulkan/pipeline.hpp"
#include "vzt/vulkan/pipeline/graphics.hpp"
#include "vzt/vulkan/program.hpp"
#include "vzt/vulkan/render_pass.hpp"

namespace vzt
{
    enum class QueueType : uint8_t;
    class Swapchain;

    struct AttachmentBuilder
    {
        ImageUsage         usage;
        Optional<Extent3D> size        = {};
        Optional<Format>   format      = {};
        uint32_t           mipLevels   = 1;
        ImageLayout        layout      = ImageLayout::Undefined;
        SampleCount        sampleCount = SampleCount::Sample1;
        ImageType          type        = ImageType::T2D;
        SharingMode        sharingMode = SharingMode::Exclusive;
        ImageTiling        tiling      = ImageTiling::Optimal;
        bool               mappable    = false;
    };

    struct StorageBuilder
    {
        std::size_t    size;
        BufferUsage    usage;
        MemoryLocation location = MemoryLocation::Device;
        bool           mappable = false;

        template <class Type>
        static StorageBuilder fromType( //
            BufferUsage usage, MemoryLocation location = MemoryLocation::Device, bool mappable = false);
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

    enum class PassType : uint8_t
    {
        Graphics = toUnderlying(QueueType::Graphics),
        Compute  = toUnderlying(QueueType::Compute)
    };

    class RenderGraph;
    class Pass
    {
      public:
        Pass(const Pass&)            = delete;
        Pass& operator=(const Pass&) = delete;

        Pass(Pass&&) noexcept   = default;
        Pass& operator=(Pass&&) = default;

        virtual ~Pass() = default;

        void addColorInput(uint32_t binding, const Handle& attachment, std::string name = "");
        void addDepthInput(uint32_t binding, const Handle& attachment, std::string name = "");
        void addColorOutput(Handle& attachment, std::string attachmentName = "", const vzt::Vec4 clearColor = {});
        void addColorInputOutput(Handle& attachment, std::string inName = "", std::string outName = "");

        void addStorageInputIndirect(const Handle& storage, std::string storageName = "",
                                     Optional<Range<std::size_t>> range = {});
        void addStorageInput(uint32_t binding, const Handle& storage, std::string storageName = "",
                             Optional<Range<std::size_t>> range = {});
        void addStorageOutput(uint32_t binding, Handle& storage, std::string storageName = "",
                              Optional<Range<std::size_t>> range = {});
        void addStorageInputOutput(uint32_t binding, Handle& storage, std::string inName = "", std::string outName = "",
                                   Optional<Range<std::size_t>> range = {});

        void setDepthInput(const Handle& depthStencil, std::string attachmentName = "");
        void setDepthOutput(Handle& depthStencil, std::string attachmentName = "", float clearValue = 1.f);

        void link(const pipeline& pipeline);

        template <class DerivedHandler, class... Args>
        void setRecordFunction(Args&&... args);
        void setRecordFunction(std::unique_ptr<RecordHandler>&& recordCallback);

        bool isDependingOn(const Pass& other) const;
        void record(uint32_t i, CommandBuffer& commands) const;

        inline std::string_view getName() const;

        inline const DescriptorLayout& getDescriptorLayout() const;
        inline DescriptorPool&         getDescriptorPool();
        inline View<RenderPass>        getRenderPass() const;

        friend RenderGraph;

      protected:
        Pass(RenderGraph& graph, std::string name, PassType type);
        virtual void compile(Format depthFormat);
        virtual void resize();

        void createRenderObjects();
        void createDescriptors();

        RenderGraph*     m_graph;
        std::string      m_name;
        PassType         m_type;
        DescriptorLayout m_descriptorLayout;

        std::unique_ptr<RecordHandler> m_recordCallback;

        struct PassAttachment
        {
            Handle        handle;
            std::string   name;
            AttachmentUse use;

            uint32_t binding = ~0u;

            Access        waitAccess   = vzt::Access::None;
            Access        targetAccess = vzt::Access::None;
            PipelineStage waitStage    = vzt::PipelineStage::None;
            PipelineStage targetStage  = vzt::PipelineStage::None;
            ImageAspect   aspect       = ImageAspect::Color;

            bool operator<(const PassAttachment& other) const { return handle.id < other.handle.id; }
        };

        struct PassStorage
        {
            Handle                       handle;
            std::string                  name;
            Optional<Range<std::size_t>> range;

            Access        waitAccess;
            Access        targetAccess;
            PipelineStage waitStage;
            PipelineStage targetStage;

            uint32_t binding = ~0u;

            bool operator<(const PassStorage& other) const { return handle.id < other.handle.id; }
        };

        std::vector<PassAttachment> m_colorInputs;
        std::vector<PassStorage>    m_storageInputs;
        Optional<PassAttachment>    m_depthInput;

        std::vector<PassAttachment> m_colorOutputs;
        std::vector<PassStorage>    m_storageOutputs;
        std::vector<PassAttachment> m_storageImageOutputs;
        Optional<PassAttachment>    m_depthOutput;

        RenderPass     m_renderPass;
        DescriptorPool m_pool;

        std::vector<FrameBuffer> m_frameBuffers; // [swapchainImageId]
        std::vector<Texture>     m_textureSaves;
        std::vector<ImageView>   m_imageViews;
    };

    class ComputePass : public Pass
    {
      public:
        ComputePass(const ComputePass&)            = delete;
        ComputePass& operator=(const ComputePass&) = delete;

        ComputePass(ComputePass&&) noexcept   = default;
        ComputePass& operator=(ComputePass&&) = default;

        ~ComputePass() = default;

        inline compute& getPipeline();

        friend RenderGraph;

      private:
        ComputePass(RenderGraph& graph, std::string name, Program&& program);
        void compile(Format depthFormat) override;

        Program m_program;
        compute m_pipeline;
    };

    class GraphicsPass : public Pass
    {
      public:
        GraphicsPass(const GraphicsPass&)            = delete;
        GraphicsPass& operator=(const GraphicsPass&) = delete;

        GraphicsPass(GraphicsPass&&) noexcept   = default;
        GraphicsPass& operator=(GraphicsPass&&) = default;

        ~GraphicsPass() = default;

        inline GraphicPipeline& getPipeline();

        friend RenderGraph;

      private:
        GraphicsPass(RenderGraph& graph, std::string name, Program&& program);
        void compile(Format depthFormat) override;
        void resize() override;

        Program         m_program;
        GraphicPipeline m_pipeline;
    };

    class RenderGraph
    {
      public:
        RenderGraph() = default;
        RenderGraph(View<Device> device, View<Swapchain> swapchain);

        // User configuration
        Handle addAttachment(AttachmentBuilder builder);
        Handle addStorage(StorageBuilder builder);

        Pass&         addPass(std::string name, PassType type = PassType::Graphics);
        ComputePass&  addCompute(std::string name, Program&& program);
        ComputePass&  addCompute(std::string name, std::vector<Shader> shaders);
        ComputePass&  addCompute(std::string name, Shader shader);
        GraphicsPass& addGraphics(std::string name, Program&& program);
        GraphicsPass& addGraphics(std::string name, std::vector<Shader> shaders);

        View<DeviceImage> getImage(uint32_t swapchainImageId, Handle handle) const;
        View<Buffer>      getStorage(uint32_t swapchainImageId, Handle handle) const;

        void setBackBuffer(Handle handle);
        bool isBackBuffer(Handle handle) const;

        // User information check
        void compile();
        void record(uint32_t i, CommandBuffer& commands);
        void resize(const Extent2D& extent);

        inline std::unique_ptr<Pass>&                             operator[](uint32_t passId);
        inline const std::unique_ptr<Pass>&                       operator[](uint32_t passId) const;
        inline uint32_t                                           size() const;
        inline std::vector<std::unique_ptr<Pass>>::iterator       begin();
        inline std::vector<std::unique_ptr<Pass>>::iterator       end();
        inline std::vector<std::unique_ptr<Pass>>::const_iterator begin() const;
        inline std::vector<std::unique_ptr<Pass>>::const_iterator end() const;

        inline View<Device> getDevice() const;

        friend Pass;
        friend ComputePass;
        friend GraphicsPass;

      private:
        Handle generateAttachmentHandle() const;
        Handle generateStorageHandle() const;

        const AttachmentBuilder& getConfiguration(Handle handle);

        std::vector<std::size_t> sort();
        void                     createRenderTarget();

        static inline std::atomic<std::size_t> m_handleCounter = 0;

        View<Device>    m_device;
        View<Swapchain> m_swapchain;

        HandleMap<AttachmentBuilder> m_attachmentBuilders;
        HandleMap<StorageBuilder>    m_storageBuilders;

        std::hash<std::size_t>             m_hash{};
        HandleMap<std::size_t>             m_handleToPhysical;
        std::vector<std::unique_ptr<Pass>> m_passes;

        std::vector<DeviceImage> m_images;   // [imageId  ]
        std::vector<Buffer>      m_storages; // [storageId]

        Optional<Handle> m_backBuffer;
    };
} // namespace vzt

#include "render_graph.inl"

#endif // VZT_UTILS_RENDERGRAPH_HPP
