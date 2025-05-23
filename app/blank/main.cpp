#include <cassert>
#include <cstdlib>

#include <vzt/Core/File.hpp>
#include <vzt/Vulkan/Command.hpp>
#include <vzt/Vulkan/Pipeline/GraphicsPipeline.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Blank";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder::standard(), surface);
    auto swapchain = vzt::Swapchain{device, surface};

    auto program = vzt::Program(device);

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    while (window.update())
    {
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
                .newLayout = vzt::ImageLayout::PresentSrcKHR,
            };
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.end();
        }

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
            device.wait();
    }

    return EXIT_SUCCESS;
}
