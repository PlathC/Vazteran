#include <cassert>
#include <cstdlib>

#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui.h>
#include <vzt/vulkan/command.hpp>
#include <vzt/vulkan/surface.hpp>
#include <vzt/vulkan/swapchain.hpp>
#include <vzt/window.hpp>

#include "vzt/vulkan/frame_buffer.hpp"
#include "vzt/vulkan/render_pass.hpp"

class Ui
{
  public:
    Ui(vzt::Window& window, vzt::View<vzt::Instance> instance, vzt::View<vzt::Device> device,
       vzt::View<vzt::Swapchain> swapchain)
        : m_device(device), m_imageNb(swapchain->getImageNb()), m_swapchain(swapchain)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();

        ImGui_ImplVulkan_LoadFunctions(
            vzt::toVulkan(instance->getAPIVersion()),
            [](const char* name, void*) { return vkGetInstanceProcAddr(volkGetLoadedInstance(), name); }, nullptr);

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForVulkan(window.getHandle());
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = instance->getHandle();
        init_info.ApiVersion                = vzt::toVulkan(instance->getAPIVersion());
        init_info.PhysicalDevice            = m_device->getHardware().getHandle();
        init_info.Device                    = m_device->getHandle();

        vzt::View<vzt::Queue> graphicsQueue = m_device->getQueue(vzt::QueueType::Graphics);
        init_info.QueueFamily               = graphicsQueue->getId();
        init_info.Queue                     = graphicsQueue->getHandle();
        init_info.PipelineCache             = VK_NULL_HANDLE;

        std::unordered_set poolTypes = {
            vzt::DescriptorType::Sampler,
            vzt::DescriptorType::CombinedSampler,
            vzt::DescriptorType::SampledImage,
            vzt::DescriptorType::StorageImage,
            vzt::DescriptorType::UniformTexelBuffer,
            vzt::DescriptorType::StorageTexelBuffer,
            vzt::DescriptorType::UniformBuffer,
            vzt::DescriptorType::StorageBuffer,
            vzt::DescriptorType::UniformBufferDynamic,
            vzt::DescriptorType::StorageBufferDynamic,
            vzt::DescriptorType::InputAttachment,
        };
        m_descriptorPool         = vzt::DescriptorPool(m_device, poolTypes, 1000 * poolTypes.size());
        init_info.DescriptorPool = m_descriptorPool.getHandle();
        init_info.Subpass        = 0;
        init_info.MinImageCount  = 2;
        init_info.ImageCount     = m_imageNb;
        init_info.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;

        m_renderPass = vzt::RenderPass(m_device);
        {
            vzt::AttachmentUse attachment{};
            attachment.format         = swapchain->getFormat();
            attachment.loadOp         = vzt::LoadOp::Load;
            attachment.storeOp        = vzt::StoreOp::Store;
            attachment.stencilLoapOp  = vzt::LoadOp::DontCare;
            attachment.stencilStoreOp = vzt::StoreOp::DontCare;
            attachment.initialLayout  = vzt::ImageLayout::ColorAttachmentOptimal;
            attachment.usedLayout     = vzt::ImageLayout::ColorAttachmentOptimal;
            attachment.finalLayout    = vzt::ImageLayout::PresentSrcKHR;
            m_renderPass.addColor(attachment);

            attachment.format         = m_device->getHardware().getDepthFormat();
            attachment.loadOp         = vzt::LoadOp::DontCare;
            attachment.storeOp        = vzt::StoreOp::DontCare;
            attachment.stencilLoapOp  = vzt::LoadOp::DontCare;
            attachment.stencilStoreOp = vzt::StoreOp::DontCare;
            attachment.initialLayout  = vzt::ImageLayout::Undefined;
            attachment.usedLayout     = vzt::ImageLayout::DepthStencilAttachmentOptimal;
            attachment.finalLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal;
            m_renderPass.setDepth(attachment);
            m_renderPass.compile();
        }
        init_info.RenderPass = m_renderPass.getHandle();

        ImGui_ImplVulkan_Init(&init_info);

        m_frameBuffers.reserve(swapchain->getImageNb());
        for (uint32_t i = 0; i < swapchain->getImageNb(); i++)
        {
            m_depthStencils.emplace_back(device, swapchain->getExtent(), vzt::ImageUsage::DepthStencilAttachment,
                                         m_device->getHardware().getDepthFormat());
            m_frameBuffers.emplace_back(m_device, swapchain->getExtent());

            vzt::FrameBuffer& frameBuffer = m_frameBuffers.back();
            frameBuffer.addAttachment(vzt::ImageView(device, swapchain->getImage(i), vzt::ImageAspect::Color));
            frameBuffer.addAttachment(vzt::ImageView(device, m_depthStencils.back(), vzt::ImageAspect::Depth));
            frameBuffer.compile(m_renderPass);
        }

        window.setEventCallback([](SDL_Event* windowEvent) { ImGui_ImplSDL3_ProcessEvent(windowEvent); });
    }

    ~Ui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void newFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void resize(vzt::Extent2D extent)
    {
        m_frameBuffers.clear();
        for (uint32_t i = 0; i < m_imageNb; i++)
        {
            m_depthStencils.emplace_back(m_device, extent, vzt::ImageUsage::DepthStencilAttachment,
                                         m_device->getHardware().getDepthFormat());
            m_frameBuffers.emplace_back(m_device, extent);

            vzt::FrameBuffer& frameBuffer = m_frameBuffers.back();
            frameBuffer.addAttachment(vzt::ImageView(m_device, m_swapchain->getImage(i), vzt::ImageAspect::Color));
            frameBuffer.addAttachment(vzt::ImageView(m_device, m_depthStencils.back(), vzt::ImageAspect::Depth));
            frameBuffer.compile(m_renderPass);
        }
    }

    void record(uint32_t imageId, vzt::CommandBuffer& commands)
    {
        ImGui::Render();

        commands.beginPass(m_renderPass, m_frameBuffers[imageId]);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commands.getHandle());
        commands.endPass();
    }

  private:
    vzt::View<vzt::Device>    m_device;
    vzt::View<vzt::Swapchain> m_swapchain;
    uint32_t                  m_imageNb;

    vzt::DescriptorPool           m_descriptorPool;
    vzt::RenderPass               m_renderPass;
    std::vector<vzt::FrameBuffer> m_frameBuffers;
    std::vector<vzt::DeviceImage> m_depthStencils;
};

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Blank";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{ApplicationName, window.getConfiguration()};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder::standard(), surface);
    auto swapchain = vzt::Swapchain{device, surface};

    auto ui      = Ui{window, instance, device, swapchain};
    auto program = vzt::Program(device);

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    while (window.update())
    {
        ui.newFrame();
        {
            const ImGuiIO& io = ImGui::GetIO();
            ImGui::ShowDemoWindow();
        }

        const auto& inputs = window.getInputs();
        if (inputs.windowResized)
            swapchain.recreate();

        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        const auto&        image    = swapchain.getImage(submission->imageId);
        vzt::CommandBuffer commands = commandPool[submission->imageId];
        {
            commands.begin();

            vzt::ImageBarrier imageBarrier{
                .image     = image,
                .oldLayout = vzt::ImageLayout::Undefined,
                .newLayout = vzt::ImageLayout::TransferDstOptimal,
            };
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.clear(image, vzt::ImageLayout::TransferDstOptimal, vzt::Vec4{1.f, 0.91f, 0.69f, 1.f});

            imageBarrier = {
                .image     = image,
                .oldLayout = vzt::ImageLayout::TransferDstOptimal,
                .newLayout = vzt::ImageLayout::ColorAttachmentOptimal,
            };
            commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::BottomOfPipe, imageBarrier);

            ui.record(submission->imageId, commands);

            commands.end();
        }

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
            device.wait();
    }

    return EXIT_SUCCESS;
}
