#ifndef VAZTERAN_MODEL_HPP
#define VAZTERAN_MODEL_HPP

#include <filesystem>
namespace fs = std::filesystem;

#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Vulkan/GpuObject.hpp"

namespace vzt {
    struct AABB {
        std::array<glm::vec3, 8> vertices;
        glm::vec3 minimum = {0.f, 0.f, 0.f};
        glm::vec3 maximum = {0.f, 0.f, 0.f};
    };

    class Model {
    public:
        Model(const fs::path& modelPath);
        const std::vector<Vertex>& Vertices() const { return m_vertices; }
        const std::vector<uint32_t>& Indices() const { return m_indices; }
        const vzt::PhongMaterial& CMat() const { return m_material; }
        vzt::PhongMaterial& Mat() { return m_material; }
        vzt::AABB BoundingBox() const;

        glm::mat4 ModelMatrix() const;
        glm::vec3& Rotation() { return m_rotation; }
        glm::vec3 CRotation() const { return m_rotation; }
        glm::vec3& Position() { return m_position; }
        glm::vec3 CPosition() const { return m_position; }

    private:
        std::vector<vzt::Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        vzt::PhongMaterial m_material;

        glm::vec3 m_position = {0.f, 0.f, 0.f};
        glm::vec3 m_rotation = {0.f, 0.f, 0.f};

        vzt::AABB m_aabb{};
    };
}

#endif //VAZTERAN_MODEL_HPP
