#include "vzt/Core/Logger.hpp"
#include "vzt/Vulkan/Core.hpp"

namespace vzt
{
    void vkCheck(VkResult result, std::string_view msg)
    {
        if (result != VK_SUCCESS)
            logger::error("{}", msg);
    }
} // namespace vzt
