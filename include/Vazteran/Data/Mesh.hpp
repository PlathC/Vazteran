
#ifndef VAZTERAN_MESH_HPP
#define VAZTERAN_MESH_HPP

#include <array>
#include <filesystem>
namespace fs = std::filesystem;
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "Vazteran/Data/Aabb.hpp"
#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Data/Vertex.hpp"

namespace vzt {
    struct SubMesh {
        uint32_t materialIndex;
        std::vector<uint32_t> vertexIndices;
    };

    class Mesh {
    public:
        Mesh(const fs::path& modelPath);
        Mesh(std::vector<vzt::Vertex> vertices, std::vector<uint32_t> vertexIndices,
             const vzt::Material& material);
        Mesh(std::vector<vzt::SubMesh> subMeshes,
             std::vector<vzt::Vertex> vertices,
             std::vector<vzt::Material> materials);

        vzt::AABB BoundingBox() const { return m_aabb; }

        // Per submesh data
        std::vector<std::vector<uint32_t>> VertexIndices() const;
        std::vector<uint32_t> MaterialIndices() const;

        // Per mesh data
        std::vector<vzt::Vertex>& Vertices() { return m_vertices; };
        std::vector<vzt::Vertex> CVertices() const { return m_vertices; };
        std::vector<vzt::Material>& Materials() { return m_materials; };
        std::vector<vzt::Material> CMaterials() const { return m_materials; };

    private:
        std::vector<vzt::SubMesh> m_subMeshes;
        std::vector<vzt::Vertex> m_vertices;
        std::vector<vzt::Material> m_materials;
        vzt::AABB m_aabb;
    };
}


#endif //VAZTERAN_MESH_HPP
