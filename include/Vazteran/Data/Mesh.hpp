
#ifndef VAZTERAN_MESH_HPP
#define VAZTERAN_MESH_HPP

#include <array>
#include <filesystem>
namespace fs = std::filesystem;
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "Vazteran/Core/Math/Aabb.hpp"
#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Framework/Vulkan/Buffer.hpp"

namespace vzt
{
	struct SubMesh
	{
		uint32_t              materialIndex;
		std::vector<uint32_t> vertexIndices;
	};

	class Mesh
	{
	  public:
		Mesh(const fs::path &modelPath);
		Mesh(std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals, std::vector<vzt::Vec2> uvs,
		     std::vector<uint32_t> vertexIndices, const vzt::Material &material);
		Mesh(std::vector<vzt::SubMesh> subMeshes, std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals,
		     std::vector<vzt::Vec2> uvs, std::vector<vzt::Material> materials);

		vzt::AABB BoundingBox() const { return m_aabb; }

		// Per submesh data
		std::vector<std::vector<uint32_t>> VertexIndices() const;
		std::vector<uint32_t>              MaterialIndices() const;

		// Per mesh data
		std::vector<vzt::Vec3>     &Vertices() { return m_vertices; };
		std::vector<vzt::Vec3>      CVertices() const { return m_vertices; };
		std::vector<vzt::Vec3>     &Normals() { return m_normals; };
		std::vector<vzt::Vec3>      CNormals() const { return m_normals; };
		std::vector<vzt::Vec2>     &Uvs() { return m_uvs; };
		std::vector<vzt::Vec2>      CUvs() const { return m_uvs; };
		std::vector<vzt::Material> &Materials() { return m_materials; };
		std::vector<vzt::Material>  CMaterials() const { return m_materials; };

	  private:
		void _createBuffers();

	  private:
		std::vector<vzt::SubMesh>  m_subMeshes;
		std::vector<vzt::Vec3>     m_vertices;
		std::vector<vzt::Vec3>     m_normals;
		std::vector<vzt::Vec2>     m_uvs;
		std::vector<vzt::Material> m_materials;
		vzt::AABB                  m_aabb;
	};
} // namespace vzt

#endif // VAZTERAN_MESH_HPP
