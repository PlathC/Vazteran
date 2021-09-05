#ifndef VAZTERAN_MODEL_HPP
#define VAZTERAN_MODEL_HPP

#include <filesystem>
namespace fs = std::filesystem;

#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Data/Mesh.hpp"
#include "Vazteran/Data/Vertex.hpp"

namespace vzt {
    class Model {
    public:
        Model(const fs::path& modelPath);
        vzt::AABB BoundingBox() const;
        vzt::Mesh& Mesh() { return m_mesh; };
        const vzt::Mesh& CMesh() const { return m_mesh; };

        glm::mat4 ModelMatrix() const;
        glm::vec3& Rotation() { return m_rotation; }
        glm::vec3 CRotation() const { return m_rotation; }
        glm::vec3& Position() { return m_position; }
        glm::vec3 CPosition() const { return m_position; }

    private:
        vzt::Mesh m_mesh;

        glm::vec3 m_position = {0.f, 0.f, 0.f};
        glm::vec3 m_rotation = {0.f, 0.f, 0.f};
    };
}

#endif //VAZTERAN_MODEL_HPP
