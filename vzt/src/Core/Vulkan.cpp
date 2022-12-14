#include "vzt/Core/Logger.hpp"
#include "vzt/Vulkan/Core.hpp"

namespace vzt
{
    void vkCheck(VkResult result, std::string_view msg)
    {
        if (result != VK_SUCCESS)
            logger::error("{}", msg);
    }

    uint32_t getAPIVersion() { return VK_API_VERSION_1_3; }
} // namespace vzt
