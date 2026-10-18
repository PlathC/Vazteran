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
#include "vzt/vulkan/query_pool.hpp"

namespace vzt
{
    enum class QueueType : uint8_t;
    class Swapchain;
    class UniformBuffer;

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
        bool               perFrame    = false;
        ImageLayout        finalLayout = ImageLayout::Undefined; // Layout after the last use (Undefined: unchanged)
    };

    struct StorageBuilder
    {
        std::size_t    size;
        BufferUsage    usage;
        MemoryLocation location = MemoryLocation::Device;
        bool           mappable = false;
        bool           perFrame = false; // Duplicate the storage once per frame

        template <class Type>
        static StorageBuilder fromType( //
            BufferUsage usage, MemoryLocation location = MemoryLocation::Device, bool mappable = false,
            bool perFrame = false);
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
        std::size_t state = 0; // Version, tracked by the graph

        struct hash
        {
            std::size_t operator()(const Handle& handle) const { return handle.id; }
        };

        bool operator==(const Handle& other) const { return id == other.id; }
        bool operator<(const Handle& other) const { return id < other.id; }
    };

    template <class Type>
    using HandleMap = std::unordered_map<Handle, Type, Handle::hash>;

    class TaskGraph;

    struct PassContext
    {
        CommandBuffer&       commands;
        const TaskGraph&     graph;
        const DescriptorSet& set;
        uint32_t             frame;

        inline View<Buffer> buffer(Handle storage) const;
    };

    class ComputePass;
    struct ComputeContext : PassContext
    {
        const ComputePass& pass;

        Vec3u groups(uint32_t x, uint32_t y = 1, uint32_t z = 1) const;
    };

    class GraphicsPass;
    struct GraphicsContext : PassContext
    {
        const GraphicsPass& pass;

        // Begins rendering on the attachments of the pass. The viewport and scissor cover `area` (full extent by
        // default)
        void begin(Optional<Rect2D> area = {}) const;
        void end() const { commands.endRendering(); }
    };

    enum class PassType : uint8_t
    {
        Graphics = toUnderlying(QueueType::Graphics),
        Compute  = toUnderlying(QueueType::Compute)
    };

    // How a pass uses a resource
    enum class Use
    {
        ColorAttachment,
        DepthAttachment,
        Sampled,
        StorageImage,
        StorageBuffer,
        Indirect,
    };

    class Pass
    {
      public:
        Pass(const Pass&)            = delete;
        Pass& operator=(const Pass&) = delete;

        Pass(Pass&&) noexcept   = default;
        Pass& operator=(Pass&&) = default;

        virtual ~Pass() = default;

        // `parameter` is the name of the resource in the shader, empty when the shader does not see it (Indirect).
        // `use` must match the handle: Sampled / StorageImage for images, StorageBuffer / Indirect for buffers.
        void read(const Handle& handle, Use use, std::string parameter = "", Optional<Range<>> range = {});
        void write(const Handle& handle, Use use, std::string parameter = "", Optional<Range<>> range = {});
        void readWrite(const Handle& handle, Use use, std::string parameter = "", Optional<Range<>> range = {});

        void bind(const UniformBuffer& buffer, std::string name);

        virtual bool    isDependingOn(const Pass& other) const;
        void            record(uint32_t i, CommandBuffer& commands) const;
        Optional<float> readDuration(uint32_t frame, QueryResultFlag flags) const;

        inline std::string_view getName() const;

        float           getDuration(uint32_t frame) const;
        Optional<float> tryGetDuration(uint32_t frame) const;

        inline DescriptorPool&        getDescriptorPool();
        inline CSpan<View<ImageView>> getColorOutputs(uint32_t frame) const;
        inline View<ImageView>        getDepth(uint32_t b) const;

        friend TaskGraph;

      protected:
        Pass(TaskGraph& graph, std::string name, PassType type);
        virtual void compile() = 0;
        virtual void resize();

        void bindBackbuffer(uint32_t frame, uint32_t image);
        void declare(const Handle& handle, Use use, bool read, bool write, std::string parameter,
                     Optional<Range<>> range);

        // Creates the descriptor sets of the pass from its pipeline's layout
        void compileDescriptors(const Program& program, const DescriptorLayout& layout);
        void createDescriptors();

        TaskGraph*  m_graph;
        std::string m_name;
        PassType    m_type;
        uint32_t    m_index = 0; // In execution order

        std::function<void(uint32_t frame, CommandBuffer&)> m_recordCallback;

        struct PassUniform
        {
            std::string         name;
            View<UniformBuffer> buffer;
            uint32_t            binding = ~0u;
        };
        std::vector<PassUniform> m_uniforms;

        struct ResourceUse
        {
            Handle            handle; // Version written by this use, or read when it only reads
            Use               use;
            bool              read      = false;
            bool              write     = false;
            std::string       parameter = {}; // Shader parameter, empty when not bound to the shader
            uint32_t          binding   = ~0u;
            Optional<Range<>> range     = {};
            ColorBlend        blend     = {.blendEnable = false};
            Optional<Vec4>    clear  = {}; // Attachments only, loaded when read, cleared with this value or discarded
            ImageAspect       aspect = ImageAspect::Color; // Resolved at compile for images
        };
        std::vector<ResourceUse> m_uses;

        // State of a resource required by a use, or reached after it
        struct ResourceState
        {
            ImageLayout   layout = ImageLayout::Undefined;
            PipelineStage stages = PipelineStage::None;
            Access        access = Access::None;
        };

        // Computed at compile from the previous use of each resource
        struct PassBarrier
        {
            Handle        handle;
            Use           use;
            ImageAspect   aspect;
            ResourceState src;
            ResourceState dst;
        };
        std::vector<PassBarrier> m_barriers;
        std::vector<PassBarrier> m_finalBarriers;

        // Descriptor data
        DescriptorPool               m_pool;
        std::vector<Texture>         m_textureSaves;
        std::vector<ImageView>       m_storageImageViews;
        std::vector<ImageView>       m_colorOutputImageViews;
        std::vector<ImageView>       m_backbufferViews; // [use][image]
        std::vector<View<ImageView>> m_colorOutputs;    // [frame][color use]
        std::vector<ImageView>       m_depthOutputImageViews;

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

        void setRecordFunction(std::function<void(ComputeContext&)> record);

        std::array<uint32_t, 3> getGroupSize() const;

        inline ComputePipeline& getPipeline();
                                operator const ComputePipeline&() const { return m_pipeline; }

        friend TaskGraph;

      private:
        ComputePass(TaskGraph& graph, std::string name, Program&& program);
        void compile() override;

        Program         m_program;
        ComputePipeline m_pipeline;
    };

    struct ColorOptions
    {
        Optional<Vec4> clear = Vec4(0.f); // Contents are discarded when empty
        ColorBlend     blend = {.blendEnable = false};
        bool           load  = false; // Keep the previous contents (read-write)
    };

    struct DepthOptions
    {
        Optional<float> clear    = 1.f;
        bool            load     = false; // Keep the previous contents (read-write)
        bool            readOnly = false;
    };

    class GraphicsPass : public Pass
    {
      public:
        GraphicsPass(const GraphicsPass&)            = delete;
        GraphicsPass& operator=(const GraphicsPass&) = delete;

        GraphicsPass(GraphicsPass&&) noexcept   = default;
        GraphicsPass& operator=(GraphicsPass&&) = default;

        ~GraphicsPass() override = default;

        void color(const Handle& attachment, ColorOptions options = {});
        void depth(const Handle& attachment, DepthOptions options = {});

        // The pass begins and ends rendering around the record function, unless it is manual (see GraphicsContext)
        void setRecordFunction(std::function<void(GraphicsContext&)> record);
        void setManualRendering(bool manual = true) { m_manualRendering = manual; }

        Extent2D      getExtent() const;
        RenderingInfo getRenderingInfo(uint32_t frame, Rect2D renderArea) const;

        inline GraphicsPipeline&        getPipeline();
                                        operator const GraphicsPipeline&() const { return m_pipeline; }
        inline GraphicsPipelineBuilder& getBuilder();

        friend TaskGraph;

      private:
        GraphicsPass(TaskGraph& graph, std::string name, Program&& program);
        void compile() override;
        void resize() override;

        Program                 m_program;
        GraphicsPipelineBuilder m_graphicsPipelineBuilder;
        GraphicsPipeline        m_pipeline;
        bool                    m_manualRendering = false;
    };

    class TaskGraph
    {
      public:
        TaskGraph() = default;
        TaskGraph(View<Device> device, uint32_t frameNb = 1);

        // User configuration
        void setBackbuffer(CSpan<View<DeviceImage>> images, uint32_t frameNb, ImageLayout finalLayout, Handle handle);
        void setBackbuffer(View<DeviceImage> image, ImageLayout finalLayout, Handle handle);
        void setBackbufferImage(uint32_t frame, uint32_t image);

        // Resources owned by the user. An external image is returned to `layout` after its last use,
        // so that the graph can be executed again. Synchronisation outside of the graph is up to the user.
        Handle external(View<Buffer> buffer);
        Handle external(View<DeviceImage> image, ImageLayout layout);

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
        void setProfiling(bool profiling) { m_profiling = profiling; }

        // User information check
        void compile();
        void record(uint32_t frame, CommandBuffer& commands);

        // Records every pass in a command buffer owned by the graph and submits it. The previous execution of
        // `frame` must be over: completion is not tracked, the caller waits (device.wait()).
        void execute(View<Queue> queue, uint32_t frame = 0);
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
        inline uint32_t     getFrameNb() const;
        inline uint32_t     getBackbufferImageNb() const;
        inline View<Device> getDevice() const;

        inline void  setUserData(void* userData);
        inline void* getUserData() const;

        friend Pass;
        friend ComputePass;
        friend GraphicsPass;

      private:
        Handle generateAttachmentHandle() const;
        Handle generateStorageHandle() const;

        const AttachmentBuilder& getConfiguration(Handle handle);

        // Version of a resource read by a pass / produced by a write: dependencies follow declaration order
        Handle readVersion(const Handle& handle) const;
        Handle writeVersion(const Handle& handle);

        std::vector<std::size_t> sort();
        void                     computeBarriers();

        // Layout, stages and accesses required by a use
        static Pass::ResourceState getState(const Pass::ResourceUse& use, PassType type);
        void                       createRenderTarget();

        static inline std::atomic<std::size_t> m_handleCounter = 0;

        View<Device> m_device;

        HandleMap<AttachmentBuilder> m_attachmentBuilders;
        HandleMap<StorageBuilder>    m_storageBuilders;

        struct ExternalImage
        {
            View<DeviceImage> image;
            ImageLayout       layout;
        };
        HandleMap<ExternalImage> m_externalImages;
        HandleMap<View<Buffer>>  m_externalBuffers;

        std::hash<std::size_t>                       m_hash{};
        HandleMap<std::size_t>                       m_handleToPhysical;
        std::vector<std::unique_ptr<Pass>>           m_passes;
        std::unordered_map<std::size_t, std::size_t> m_versions; // [handle id]

        std::vector<std::vector<DeviceImage>> m_images;   // [imageId][copy]
        std::vector<std::vector<Buffer>>      m_storages; // [storageId][copy]

        Optional<Handle>      m_backbuffer;
        uint32_t              m_frameNb = 1;
        std::vector<uint32_t> m_frameImages{0}; // [frame] -> index in m_externalBackbuffers
        Format                m_backbufferFormat;
        Extent2D              m_backbufferExtent;
        ImageLayout           m_backbufferLayout;

        std::vector<View<DeviceImage>> m_externalBackbuffers;

        bool      m_profiling = false;
        QueryPool m_queries; // [frame][pass][begin, end]

        View<Queue> m_queue;
        CommandPool m_commandPool; // One buffer per frame, created by the first execute

        void* m_userData = nullptr;
    };

    // TaskGraph presenting to a swapchain
    class RenderGraph : public TaskGraph
    {
      public:
        RenderGraph() = default;
        RenderGraph(View<Device> device) : TaskGraph(device) {}

        using TaskGraph::setBackbuffer;
        void setBackbuffer(View<Swapchain> swapchain, Handle handle);
    };
} // namespace vzt

#include "render_graph.inl"

#endif // VZT_UTILS_RENDERGRAPH_HPP
