#include "vzt/vulkan/setup.hpp"

#include <stdexcept>

#include "vzt/core/logger.hpp"

namespace vzt
{
    void vkCheck(VkResult result, std::string_view msg)
    {
        if (result < VK_SUCCESS)
            throw std::runtime_error(fmt::format("{} (VkResult {})", msg, static_cast<int>(result)));
    }
} // namespace vzt
