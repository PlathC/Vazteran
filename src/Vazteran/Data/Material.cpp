#include "Vazteran/Data/Material.hpp"

namespace vzt {
    Material::Material(glm::vec4 albedo) :
            m_albedo(Image({
                static_cast<uint8_t>(albedo.r * 255),
                static_cast<uint8_t>(albedo.g * 255),
                static_cast<uint8_t>(albedo.b * 255),
                static_cast<uint8_t>(albedo.a * 255)
            }, 1, 1, 4))
    {
    }

    Material::Material(Image albedo):
            m_albedo(std::move(albedo)){
    }
}