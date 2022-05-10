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

	struct Mesh
	{
		Mesh(const fs::path& modelPath);
		Mesh(std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals, std::vector<vzt::Vec2> uvs,
		     std::vector<uint32_t> vertexIndices, const vzt::Material& material);
		Mesh(std::vector<vzt::SubMesh> subMeshes, std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals,
		     std::vector<vzt::Vec2> uvs, std::vector<vzt::Material> materials);

		// Per submesh data
		std::vector<std::vector<uint32_t>> getVertexIndices() const;
		std::vector<uint32_t>              getMaterialIndices() const;

		std::vector<vzt::SubMesh>  subMeshes;
		std::vector<vzt::Vec3>     vertices;
		std::vector<vzt::Vec3>     normals;
		std::vector<vzt::Vec2>     uvs;
		std::vector<vzt::Material> materials;
		vzt::AABB                  aabb;
	};
} // namespace vzt

#endif // VAZTERAN_MESH_HPP
