#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include <glm/glm.hpp>

#include "Vazteran/Data/Image.hpp"

namespace vzt {
    struct PhongMaterial {
        Image ambient;
        Image diffuse;
        Image specular;
    };
}

#endif //VAZTERAN_MATERIAL_HPP
