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
#include "vzt/vulkan/pipeline.hpp"
#include "vzt/vulkan/program.hpp"

namespace vzt
{
    enum class QueueType : uint8_t;
    class Swapchain;

    struct AttachmentUse
    {
        Format      format;
        ImageLayout initialLayout;
        ImageLayout finalLayout;
        ImageLayout usedLayout;
    };

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

    struct Attachment
    {
        View<ImageView> image;
        ImageLayout     layout     = ImageLayout::Undefined;
        LoadOp          loadOp     = LoadOp::Clear;
        StoreOp         storeOp    = StoreOp::Store;
        Vec4            clearValue = vzt::Vec4{0.f};
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

        void addStorageInputIndirect(const Handle& storage, std::string storageName = "", Optional<Range<>> range = {});
        void addStorageInput(uint32_t binding, const Handle& storage, std::string storageName = "",
                             Optional<Range<>> range = {});
        void addStorageOutput(uint32_t binding, Handle& storage, std::string storageName = "",
                              Optional<Range<>> range = {});
        void addStorageInputOutput(uint32_t binding, Handle& storage, std::string inName = "", std::string outName = "",
                                   Optional<Range<>> range = {});
        void addColorTextureInput(uint32_t binding, const Handle& handle, std::string name = "");
        void addDepthTextureInput(uint32_t binding, const Handle& handle, std::string name = "");

        void link(const Pipeline& pipeline);

        template <class DerivedHandler, class... Args>
        void setRecordFunction(Args&&... args);
        void setRecordFunction(std::unique_ptr<RecordHandler>&& recordCallback);

        virtual bool isDependingOn(const Pass& other) const;
        void         record(uint32_t i, CommandBuffer& commands) const;

        inline std::string_view getName() const;

        inline DescriptorLayout& getDescriptorLayout();
        inline DescriptorPool&   getDescriptorPool();
        inline CSpan<ImageView>  getColorOutputs(uint32_t b) const;
        inline View<ImageView>   getDepth(uint32_t b) const;

        friend RenderGraph;

      protected:
        Pass(RenderGraph& graph, std::string name, PassType type);
        virtual void compile();
        virtual void resize();

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

            Access        waitAccess   = Access::None;
            Access        targetAccess = Access::None;
            PipelineStage waitStage    = PipelineStage::None;
            PipelineStage targetStage  = PipelineStage::None;
            ImageAspect   aspect       = ImageAspect::Color;

            ColorBlend blend = {.blendEnable = false};

            bool operator<(const PassAttachment& other) const { return handle.id < other.handle.id; }
        };

        struct PassStorage
        {
            Handle            handle;
            std::string       name;
            Optional<Range<>> range;

            Access        waitAccess;
            Access        targetAccess;
            PipelineStage waitStage;
            PipelineStage targetStage;

            uint32_t binding = ~0u;

            bool operator<(const PassStorage& other) const { return handle.id < other.handle.id; }
        };

        // Shader read/write
        std::vector<PassStorage> m_storageInputs;
        std::vector<PassStorage> m_storageOutputs;

        std::vector<PassAttachment> m_textureInputs;
        std::vector<PassAttachment> m_storageImageOutputs;

        // Graphics pass read/write
        std::vector<PassAttachment> m_colorInputs;
        std::vector<PassAttachment> m_colorOutputs;

        Optional<PassAttachment> m_depthInput;
        Optional<PassAttachment> m_depthOutput;

        // Descriptor data
        DescriptorPool         m_pool;
        std::vector<Texture>   m_textureSaves;
        std::vector<ImageView> m_storageImageViews;
        std::vector<ImageView> m_colorOutputImageViews;
        std::vector<ImageView> m_depthOutputImageViews;

        friend class GraphicsPass;
    };

    class ComputePass : public Pass
    {
      public:
        ComputePass(const ComputePass&)            = delete;
        ComputePass& operator=(const ComputePass&) = delete;

        ComputePass(ComputePass&&) noexcept   = default;
        ComputePass& operator=(ComputePass&&) = default;

        ~ComputePass() override = default;

        inline ComputePipeline& getPipeline();

        friend RenderGraph;

      private:
        ComputePass(RenderGraph& graph, std::string name, Program&& program);
        void compile() override;

        Program         m_program;
        ComputePipeline m_pipeline;
    };

    class GraphicsPass : public Pass
    {
      public:
        GraphicsPass(const GraphicsPass&)            = delete;
        GraphicsPass& operator=(const GraphicsPass&) = delete;

        GraphicsPass(GraphicsPass&&) noexcept   = default;
        GraphicsPass& operator=(GraphicsPass&&) = default;

        ~GraphicsPass() override = default;

        void setDepthInput(const Handle& depthStencil, std::string attachmentName = "");
        void setDepthOutput(Handle& depthStencil, std::string attachmentName = "");
        void setDepthInputOutput(Handle& depthStencil, std::string attachmentName = "");

        void addColorOutput(Handle& attachment, std::string attachmentName = "",
                            ColorBlend blend = {.blendEnable = false});
        void addColorInputOutput(Handle& attachment, std::string inName = "", std::string outName = "",
                                 ColorBlend blend = {.blendEnable = false});

        inline GraphicsPipeline&        getPipeline();
        inline GraphicsPipelineBuilder& getBuilder();

        friend RenderGraph;

      private:
        GraphicsPass(RenderGraph& graph, std::string name, Program&& program);
        void compile() override;
        void resize() override;

        Program                 m_program;
        GraphicsPipelineBuilder m_graphicsPipelineBuilder;
        GraphicsPipeline        m_pipeline;
    };

    class RenderGraph
    {
      public:
        RenderGraph() = default;
        RenderGraph(View<Device> device);

        // User configuration
        void setBackbuffer(View<DeviceImage> image, ImageLayout finalLayout, Handle handle);
        void setBackbuffer(View<Swapchain> swapchain, Handle handle);

        Handle addAttachment(AttachmentBuilder builder);
        Handle addStorage(StorageBuilder builder);

        ComputePass&  addCompute(std::string name, Program&& program);
        ComputePass&  addCompute(std::string name, std::vector<Shader> shaders);
        ComputePass&  addCompute(std::string name, Shader shader);
        GraphicsPass& addGraphics(std::string name, Program&& program);
        GraphicsPass& addGraphics(std::string name, std::vector<Shader> shaders);

        View<DeviceImage> getImage(uint32_t backbufferId, Handle handle) const;
        View<Buffer>      getStorage(uint32_t backbufferId, Handle handle) const;

        bool isBackBuffer(Handle handle) const;

        // User information check
        void compile();
        void record(uint32_t i, CommandBuffer& commands);
        void resize(const Extent2D& extent);

        inline std::unique_ptr<Pass>&       operator[](uint32_t passId);
        inline const std::unique_ptr<Pass>& operator[](uint32_t passId) const;
        inline uint32_t                     size() const;

        inline std::vector<std::unique_ptr<Pass>>::iterator       begin();
        inline std::vector<std::unique_ptr<Pass>>::iterator       end();
        inline std::vector<std::unique_ptr<Pass>>::const_iterator begin() const;
        inline std::vector<std::unique_ptr<Pass>>::const_iterator end() const;

        inline Format       getBackbufferFormat() const;
        inline Extent2D     getBackbufferExtent() const;
        inline uint32_t     getBackbufferNb() const;
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

        View<Device> m_device;

        HandleMap<AttachmentBuilder> m_attachmentBuilders;
        HandleMap<StorageBuilder>    m_storageBuilders;

        std::hash<std::size_t>             m_hash{};
        HandleMap<std::size_t>             m_handleToPhysical;
        std::vector<std::unique_ptr<Pass>> m_passes;

        std::vector<DeviceImage> m_images;   // [imageId  ]
        std::vector<Buffer>      m_storages; // [storageId]

        Optional<Handle> m_backbuffer;
        uint32_t         m_backbufferNb = 1;
        Format           m_backbufferFormat;
        Extent2D         m_backbufferExtent;
        ImageLayout      m_backbufferLayout;

        std::vector<View<DeviceImage>> m_externalBackbuffers;
    };
} // namespace vzt

#include "render_graph.inl"

#endif // VZT_UTILS_RENDERGRAPH_HPP
