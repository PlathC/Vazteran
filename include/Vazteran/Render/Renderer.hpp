//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#ifndef VAZTERAN_RENDERER_HPP
#define VAZTERAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace vzt
{
    class Renderer
    {
    public:
        Renderer(const std::vector<const char*>& extensions);

        ~Renderer();
    private:
        VkInstance m_vkInstance;
    };
}



#endif //VAZTERAN_RENDERER_HPP
