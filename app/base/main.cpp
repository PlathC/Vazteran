#include <cstdlib>

#include <vzt/Command.hpp>
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

    const auto& images      = swapchain.getImages();
    auto        commandPool = vzt::CommandPool(device, device.getQueue(vzt::QueueType::Graphics), images.size());

    while (window.update())
    {
        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        vzt::CommandBuffer commands = commandPool[submission->imageId];

        VkImageMemoryBarrier transition{};
        transition.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        transition.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
        transition.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        transition.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        transition.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        transition.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        transition.subresourceRange.baseMipLevel   = 0;
        transition.subresourceRange.levelCount     = 1;
        transition.subresourceRange.baseArrayLayer = 0;
        transition.subresourceRange.layerCount     = 1;

        transition.image = images[submission->imageId];
        vkCmdPipelineBarrier(commands.getHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &transition);

        commands.flush();

        VkSubmitInfo emptySubmit{};
        emptySubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        constexpr VkPipelineStageFlags waitStage =
            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        emptySubmit.pWaitDstStageMask    = &waitStage;
        emptySubmit.waitSemaphoreCount   = 1;
        emptySubmit.pWaitSemaphores      = &submission->imageAvailable;
        emptySubmit.signalSemaphoreCount = 1;
        emptySubmit.pSignalSemaphores    = &submission->renderComplete;
        emptySubmit.commandBufferCount   = 1;
        emptySubmit.pCommandBuffers      = &commands.getHandle();

        vkResetFences(device.getHandle(), 1, &submission->frameComplete);

        auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
        vzt::vkCheck(vkQueueSubmit(graphicsQueue->getHandle(), 1, &emptySubmit, submission->frameComplete),
                     "Failed to submit graphics queue");

        if (!swapchain.present(*submission))
            continue;
    }

    return EXIT_SUCCESS;
}
