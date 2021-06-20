
#ifndef VAZTERAN_CAMERA_HPP
#define VAZTERAN_CAMERA_HPP

#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Data/Aabb.hpp"

namespace vzt {
    struct Camera {
        glm::vec3 position;
        glm::vec3 target;
        float aspectRatio;

        float fov = glm::radians(45.f);
        float nearClipping = 0.1f;
        float farClipping = 100.f;

        glm::vec3 upVector = glm::vec3(0.f, 0.f, 1.f);

        glm::mat4 View() const { return glm::lookAt(position, target, upVector); }
        glm::mat4 Projection() const { return glm::perspective(fov, aspectRatio, nearClipping, farClipping); }

        static Camera FromModel(const vzt::AABB& referenceBoundingBox, float aspectRatio, float fov = glm::radians(45.f),
                                glm::vec3 upVector = glm::vec3(0.f, 0.f, 1.f), float nearClipping = 0.1f,
                                float farClipping = 1000.f);
    };
}

#endif //VAZTERAN_CAMERA_HPP
