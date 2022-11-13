#include <cassert>
#include <cstdlib>

#include <vzt/Core/File.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Vulkan/Attachment.hpp>
#include <vzt/Vulkan/Command.hpp>
#include <vzt/Vulkan/Pipeline.hpp>
#include <vzt/Vulkan/RenderPass.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Blank";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder{}, surface);
    auto hardware  = device.getHardware();
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    auto compiler = vzt::Compiler();

    auto program = vzt::Program(device);
    program.setShader(compiler.compile("shaders/triangle.vert", vzt::ShaderStage::Vertex));
    program.setShader(compiler.compile("shaders/triangle.frag", vzt::ShaderStage::Fragment));

    auto pipeline = vzt::Pipeline(device);
    pipeline.addAttachment();

    std::vector<vzt::RenderPass> passes{};
    passes.reserve(swapchain.getImageNb());
    for (std::size_t i = 0; i < swapchain.getImageNb(); i++)
    {
        passes.emplace_back(device);

        vzt::RenderPass& pass = passes.back();

        // clang-format off
        pass.addInput(vzt::AttachmentUse{
            vzt::Format::R16G16B16A16SFloat,
            vzt::ImageLayout::Undefined,
            vzt::ImageLayout::ColorAttachmentOptimal,
            vzt::ImageLayout::ColorAttachmentOptimal
        });
        // clang-format on

        // clang-format off
        vzt::AttachmentUse depth{
            hardware.getDepthFormat(),
            vzt::ImageLayout::Undefined,
            vzt::ImageLayout::DepthStencilAttachmentOptimal,
            vzt::ImageLayout::DepthStencilAttachmentOptimal
        };
        depth.clearValue = vzt::Vec4{1.f, 0.f, 0.f, 0.f};
        pass.addInput(std::move(depth));
        // clang-format on
    }

    pipeline.compile(passes.back());

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    while (window.update())
    {
        const auto& inputs = window.getInputs();
        if (inputs.windowResized)
            swapchain.setExtent(inputs.windowSize);

        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        const auto&        image    = swapchain.getImage(submission->imageId);
        vzt::CommandBuffer commands = commandPool[submission->imageId];
        {
            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::Undefined;
            imageBarrier.newLayout = vzt::ImageLayout::TransferDstOptimal;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.clear(image, vzt::ImageLayout::TransferDstOptimal, vzt::Vec4{1.f, 0.f, 0.f, 0.f});

            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::TransferDstOptimal;
            imageBarrier.newLayout = vzt::ImageLayout::PresentSrcKHR;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);
        }

        graphicsQueue->submit(commands, *submission);
        if (swapchain.present())
            device.wait();
    }

    return EXIT_SUCCESS;
}
