#include <cassert>
#include <cstdlib>

#include <vzt/Command.hpp>
#include <vzt/Core/Logger.hpp>
#include <vzt/Surface.hpp>
#include <vzt/Swapchain.hpp>

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Blank";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceConfiguration{}, surface);
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    while (window.update())
    {
        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        const auto& images = swapchain.getImages();
        {
            vzt::CommandBuffer commands = commandPool[submission->imageId];

            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = images[submission->imageId];
            imageBarrier.oldLayout = vzt::ImageLayout::Undefined;
            imageBarrier.newLayout = vzt::ImageLayout::PresentSrcKHR;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, std::move(imageBarrier));

            graphicsQueue->submit(commands, *submission);
        }

        if (!swapchain.present())
            continue;
    }

    return EXIT_SUCCESS;
}
