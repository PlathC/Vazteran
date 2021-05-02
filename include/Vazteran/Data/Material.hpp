#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include <glm/glm.hpp>

#include "Vazteran/Data/Image.hpp"

namespace vzt {
    class Material {
    public:
        Material(glm::vec4 albedo);
        Material(Image albedo);

        const Image& Albedo() const { return m_albedo; }

    private:
        Image m_albedo;
    };
}

#endif //VAZTERAN_MATERIAL_HPP
