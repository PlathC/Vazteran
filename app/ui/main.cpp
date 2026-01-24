#include <cassert>
#include <cstdlib>

//
#include <vzt/core/disable_warnings.hpp>
//
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui.h>
//
#include <vzt/core/enable_warnings.hpp>
//

#include <vzt/vulkan/command.hpp>
#include <vzt/vulkan/surface.hpp>
#include <vzt/vulkan/swapchain.hpp>
#include <vzt/window.hpp>

class Ui
{
  public:
    Ui(vzt::Window& window, vzt::View<vzt::Instance> instance, vzt::View<vzt::Device> device,
       vzt::View<vzt::Swapchain> swapchain)
        : m_device(device), m_swapchain(swapchain), m_imageNb(swapchain->getImageNb())
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

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
        init_info.UseDynamicRendering       = true;

        VkFormat colorAttachmentFormat        = vzt::toVulkan(swapchain->getFormat());
        init_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount    = 1;
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorAttachmentFormat;

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

        m_descriptorPool = {
            m_device,
            vzt::DescriptorPoolBuilder{.descriptorTypes = poolTypes}.addFlag(
                vzt::DescriptorPoolCreateFlag::FreeDescriptorSet),
        };
        init_info.DescriptorPool = m_descriptorPool.getHandle();
        init_info.Subpass        = 0;
        init_info.MinImageCount  = 2;
        init_info.ImageCount     = m_imageNb;
        init_info.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);

        window.setEventCallback([](SDL_Event* windowEvent) { ImGui_ImplSDL3_ProcessEvent(windowEvent); });

        resize();
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

    void resize()
    {
        m_imageViews.resize(m_swapchain->getImageNb());
        for (uint32_t i = 0; i < m_swapchain->getImageNb(); i++)
            m_imageViews[i] = vzt::ImageView(m_device, m_swapchain->getImage(i), vzt::ImageAspect::Color);
    }

    void record(uint32_t imageId, vzt::CommandBuffer& commands)
    {
        ImGui::Render();

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;

        colorAttachment.imageView   = m_imageViews[imageId].getHandle();
        colorAttachment.imageLayout = vzt::toVulkan(vzt::ImageLayout::ColorAttachmentOptimal);
        colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

        const vzt::Extent2D& extent = m_swapchain->getExtent();
        commands.beginRendering({
            .renderArea       = {0, 0, extent.width, extent.height},
            .colorAttachments = {{
                .view       = m_imageViews[imageId],
                .layout     = vzt::ImageLayout::ColorAttachmentOptimal,
                .clearValue = vzt::Vec4(1.f, 0.91f, 0.69f, 1.f),
            }},
        });

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commands.getHandle());
        commands.endRendering();
    }

  private:
    vzt::View<vzt::Device>    m_device;
    vzt::View<vzt::Swapchain> m_swapchain;
    uint32_t                  m_imageNb;

    vzt::DescriptorPool         m_descriptorPool;
    std::vector<vzt::ImageView> m_imageViews;
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
            // const ImGuiIO& io = ImGui::GetIO();
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

            imageBarrier = {
                .image     = image,
                .oldLayout = vzt::ImageLayout::ColorAttachmentOptimal,
                .newLayout = vzt::ImageLayout::PresentSrcKHR,
            };
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::BottomOfPipe, imageBarrier);

            commands.end();
        }

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
        {
            device.wait();
            ui.resize();
        }
    }

    return EXIT_SUCCESS;
}
