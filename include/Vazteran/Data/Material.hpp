#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include <glm/glm.hpp>

#include "Vazteran/Data/Types.hpp"
#include "Vazteran/Data/Image.hpp"

namespace vzt {
    struct Material {
        vzt::Image ambientMap;
        vzt::Image diffuseMap;
        vzt::Image specularMap;

        vzt::Color ambientColor;
        vzt::Color diffuseColor;
        vzt::Color specularColor;
        float shininess;
    };
}

#endif //VAZTERAN_MATERIAL_HPP
