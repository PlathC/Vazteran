#include <set>
#include <utility>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vazteran/Data/Mesh.hpp"

namespace vzt
{
	Mesh::Mesh(const fs::path &modelPath)
	{
		tinyobj::attrib_t                attrib;
		std::vector<tinyobj::shape_t>    shapes;
		std::vector<tinyobj::material_t> materials;
		std::string                      errorMessage;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &errorMessage, modelPath.string().c_str(),
		                      (modelPath.parent_path().string() + "/").c_str()))
		{
			throw std::runtime_error(errorMessage);
		}

		std::unordered_map<int, uint32_t> tinyObjToVazteranVertexIndices;
		std::unordered_map<int, uint32_t> tinyObjToVazteranMaterialIndices;

		// First material is default one in case of shape "-1" material index
		m_materials.emplace_back();
		for (int materialId = 0; materialId < materials.size(); materialId++)
		{
			tinyObjToVazteranMaterialIndices[materialId] = static_cast<uint32_t>(m_materials.size());
			auto currentMaterial                         = materials[materialId];

			if (currentMaterial.name == "None")
			{
				m_materials.emplace_back();
			}
			else
			{
				vzt::Image ambientTexture{};
				vzt::Image diffuseTexture{};
				vzt::Image specularTexture{};
				if (fs::exists(currentMaterial.ambient_texname))
					ambientTexture = vzt::Image(currentMaterial.ambient_texname);
				if (fs::exists(currentMaterial.diffuse_texname))
					diffuseTexture = vzt::Image(currentMaterial.diffuse_texname);
				if (fs::exists(currentMaterial.specular_texname))
					specularTexture = vzt::Image(currentMaterial.specular_texname);

				m_materials.emplace_back(vzt::Material{
				    ambientTexture, diffuseTexture, specularTexture,
				    vzt::Vec4{currentMaterial.ambient[0], currentMaterial.ambient[1], currentMaterial.ambient[2], 1.f},
				    vzt::Vec4{currentMaterial.diffuse[0], currentMaterial.diffuse[1], currentMaterial.diffuse[2], 1.f},
				    vzt::Vec4{currentMaterial.specular[0], currentMaterial.specular[1], currentMaterial.specular[2],
				              1.f},
				    currentMaterial.shininess});
			}
		}

		for (const auto &shape : shapes)
		{
			std::size_t index_offset = 0;

			// Loop over vertices in the face.
			std::vector<uint32_t> subMeshVertexIndices;
			subMeshVertexIndices.reserve(shape.mesh.indices.size());
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
			{
				std::size_t fv = static_cast<size_t>(shape.mesh.num_face_vertices[f]);
				for (std::size_t vertexIndex = 0; vertexIndex < fv; vertexIndex++)
				{
					tinyobj::index_t index = shape.mesh.indices[index_offset + vertexIndex];
					if (tinyObjToVazteranVertexIndices.find(index.vertex_index) == tinyObjToVazteranVertexIndices.end())
					{
						tinyObjToVazteranVertexIndices[index.vertex_index] = static_cast<uint32_t>(m_vertices.size());
						vzt::Vec3 vertex{attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 0],
						                 attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 1],
						                 attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 2]};

						// Check if `normal_index` is zero or positive. negative = no normal data
						vzt::Vec3 normal{};
						if (index.normal_index >= 0)
						{
							normal.x = attrib.normals[3 * static_cast<size_t>(index.normal_index) + 0];
							normal.y = attrib.normals[3 * static_cast<size_t>(index.normal_index) + 1];
							normal.z = attrib.normals[3 * static_cast<size_t>(index.normal_index) + 2];
						}

						// Check if `texcoord_index` is zero or positive. negative = no texcoord data
						vzt::Vec2 texCoord{};
						if (index.texcoord_index >= 0)
						{
							texCoord.x = attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 0];
							texCoord.y = attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 1];
						}

						m_vertices.emplace_back(vertex);
						m_normals.emplace_back(normal);
						m_uvs.emplace_back(texCoord);
					}
					subMeshVertexIndices.push_back(tinyObjToVazteranVertexIndices[index.vertex_index]);
				}
				index_offset += fv;
			}

			// Handle only one subMesh material
			auto materialId = shape.mesh.material_ids[0];
			m_subMeshes.push_back({tinyObjToVazteranMaterialIndices[materialId], subMeshVertexIndices});
		}

		m_aabb = vzt::AABB(m_vertices);
	}

	Mesh::Mesh(std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals, std::vector<vzt::Vec2> uvs,
	           std::vector<uint32_t> vertexIndices, const vzt::Material &material)
	    : m_subMeshes({vzt::SubMesh{0, std::move(vertexIndices)}}), m_vertices(std::move(vertices)),
	      m_normals(std::move(normals)), m_uvs(std::move(uvs)), m_materials({material})
	{
	}

	Mesh::Mesh(std::vector<vzt::SubMesh> subMeshes, std::vector<vzt::Vec3> vertices, std::vector<vzt::Vec3> normals,
	           std::vector<vzt::Vec2> uvs, std::vector<vzt::Material> materials)
	    : m_subMeshes(std::move(subMeshes)), m_vertices(std::move(vertices)), m_normals(std::move(normals)),
	      m_uvs(std::move(uvs)), m_materials(std::move(materials)), m_aabb(m_vertices)
	{
	}

	std::vector<std::vector<uint32_t>> Mesh::VertexIndices() const
	{
		std::vector<std::vector<uint32_t>> indices;
		indices.reserve(m_subMeshes.size());
		for (const auto &subMesh : m_subMeshes)
		{
			indices.emplace_back(subMesh.vertexIndices.begin(), subMesh.vertexIndices.end());
		}
		return indices;
	}

	std::vector<uint32_t> Mesh::MaterialIndices() const
	{
		std::vector<uint32_t> indices(m_subMeshes.size());
		for (const auto &subMesh : m_subMeshes)
		{
			indices.emplace_back(subMesh.materialIndex);
		}
		return indices;
	}

} // namespace vzt
