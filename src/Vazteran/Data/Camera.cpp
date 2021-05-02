#include "Vazteran/Data/Camera.hpp"

namespace vzt {
    Camera Camera::FromModel(const Model& referenceModel, float aspectRatio, float fov, glm::vec3 upVector,
                             float nearClipping, float farClipping) {
        const auto aabb = referenceModel.BoundingBox();
        glm::length_t upIndex = 0;
        if (upVector[1] == 1.f) { upIndex = 1; }
        else if (upVector[2] == 1.f) { upIndex = 2; }

        const float modelHeight = aabb.maximum[upIndex] - aabb.minimum[upIndex];
        const auto modelCenter = (aabb.maximum + aabb.minimum) / 2.f;
        const float distance = modelHeight / 2.f / std::tan(fov / 2.f);
        return Camera {
                modelCenter - glm::vec3(0., 2., 0.) * distance,
                modelCenter,
            aspectRatio,
            fov,
            nearClipping,
            farClipping
        };
    }
}
