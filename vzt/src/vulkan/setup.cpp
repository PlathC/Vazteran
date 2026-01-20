#include "vzt/core/logger.hpp"
#include "vzt/vulkan/setup.hpp"

namespace vzt
{
    void vkCheck(VkResult result, std::string_view msg)
    {
        if (result != VK_SUCCESS)
            logger::error("{}", msg);
    }
} // namespace vzt
