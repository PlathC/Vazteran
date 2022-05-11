#include "Vazteran/Data/Mesh.hpp"

namespace vzt
{
	std::vector<std::vector<uint32_t>> Mesh::getVertexIndices() const
	{
		std::vector<std::vector<uint32_t>> indices;
		indices.reserve(subMeshes.size());
		for (const auto& subMesh : subMeshes)
			indices.emplace_back(subMesh.vertexIndices.begin(), subMesh.vertexIndices.end());
		return indices;
	}

	std::vector<uint32_t> Mesh::getMaterialIndices() const
	{
		std::vector<uint32_t> indices;
		indices.reserve(subMeshes.size());
		for (const auto& subMesh : subMeshes)
			indices.emplace_back(subMesh.materialIndex);
		return indices;
	}

} // namespace vzt
