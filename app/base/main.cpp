#include <cstdlib>

#include <vzt/Surface.hpp>
#include <vzt/Swapchain.hpp>

int main(int argc, char** argv)
{
    const std::string ApplicationName = "Vazteran Base";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceConfiguration{}, surface);
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    while (true)
    {
        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        VkSubmitInfo emptySubmit{};
        emptySubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        constexpr VkPipelineStageFlags waitStage =
            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        emptySubmit.pWaitDstStageMask    = &waitStage;
        emptySubmit.waitSemaphoreCount   = 1;
        emptySubmit.pWaitSemaphores      = &submission->imageAvailable;
        emptySubmit.signalSemaphoreCount = 1;
        emptySubmit.pSignalSemaphores    = &submission->renderComplete;
        emptySubmit.commandBufferCount   = 0;
        emptySubmit.pCommandBuffers      = nullptr;

        vkResetFences(device.getHandle(), 1, &submission->frameComplete);

        auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
        vzt::vkCheck(vkQueueSubmit(graphicsQueue->getHandle(), 1, &emptySubmit, submission->frameComplete),
                     "Failed to submit graphics queue");

        if (!swapchain.present(*submission))
            continue;
    }

    return EXIT_SUCCESS;
}
