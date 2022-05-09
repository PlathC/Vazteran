
#ifndef VAZTERAN_MESH_HPP
#define VAZTERAN_MESH_HPP

#include <array>
#include <filesystem>
namespace fs = std::filesystem;
#include <vector>

#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Math/Aabb.hpp"

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
		Mesh(const fs::path& modelPath);
		Mesh(std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals, std::vector<vzt::Vec2> uvs,
		     std::vector<uint32_t> vertexIndices, const vzt::Material& material);
		Mesh(std::vector<vzt::SubMesh> subMeshes, std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals,
		     std::vector<vzt::Vec2> uvs, std::vector<vzt::Material> materials);

		vzt::AABB boundingBox() const { return m_aabb; }

		// Per submesh data
		std::vector<std::vector<uint32_t>> getVertexIndices() const;
		std::vector<uint32_t>              getMaterialIndices() const;

		// Per mesh data
		std::vector<vzt::Vec3>&     vertices() { return m_vertices; };
		std::vector<vzt::Vec3>      cVertices() const { return m_vertices; };
		std::vector<vzt::Vec3>&     normals() { return m_normals; };
		std::vector<vzt::Vec3>      cNormals() const { return m_normals; };
		std::vector<vzt::Vec2>&     uvs() { return m_uvs; };
		std::vector<vzt::Vec2>      cUvs() const { return m_uvs; };
		std::vector<vzt::Material>& materials() { return m_materials; };
		std::vector<vzt::Material>  cMaterials() const { return m_materials; };

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
