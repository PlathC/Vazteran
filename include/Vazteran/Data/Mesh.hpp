
#ifndef VAZTERAN_MESH_HPP
#define VAZTERAN_MESH_HPP

#include <array>
#include <filesystem>
namespace fs = std::filesystem;
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Data/Vertex.hpp"

namespace vzt {
    struct AABB {
        std::array<glm::vec3, 8> vertices;
        glm::vec3 minimum = {0.f, 0.f, 0.f};
        glm::vec3 maximum = {1.f, 1.f, 1.f};
    };

    struct SubMesh {
        uint32_t materialIndex;
        std::vector<uint32_t> vertexIndices;
    };

    class Mesh {
    public:
        Mesh(const fs::path& modelPath);
        Mesh(const std::vector<vzt::Vertex>& vertices, const std::vector<uint32_t> vertexIndices,
             const vzt::Material& material);
        Mesh(std::vector<vzt::SubMesh> subMeshes,
             std::vector<vzt::Vertex> vertices,
             std::vector<vzt::Material> materials);

        vzt::AABB BoundingBox() const { return m_aabb; }

        // Per submesh data
        std::vector<std::vector<uint32_t>> VertexIndices() const;
        std::vector<uint32_t> MaterialIndices() const;

        // Per mesh data
        std::vector<vzt::Vertex> Vertices() const;
        std::vector<vzt::Material> Materials() const;

    private:
        void ComputeBoundingBox();

    private:
        std::vector<vzt::SubMesh> m_subMeshes;
        std::vector<vzt::Vertex> m_vertices;
        std::vector<vzt::Material> m_materials;
        vzt::AABB m_aabb{};
    };
}


#endif //VAZTERAN_MESH_HPP
