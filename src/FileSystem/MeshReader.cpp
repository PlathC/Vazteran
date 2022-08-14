#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/FileSystem/MeshReader.hpp"

namespace vzt
{
	Mesh readObj(const Path& path)
	{
		tinyobj::ObjReader       reader;
		tinyobj::ObjReaderConfig config;
		config.mtl_search_path = path.parent_path().string();
		reader.ParseFromFile(path.string());

		const tinyobj::attrib_t&             attributes = reader.GetAttrib();
		const std::vector<tinyobj::shape_t>& shapes     = reader.GetShapes();
		std::vector<tinyobj::material_t>     materials  = reader.GetMaterials();

		if (!reader.Valid())
		{
			const std::string warnings = reader.Warning();
			const std::string errors   = reader.Error();

			VZT_WARNING(warnings);
			VZT_ERROR(errors);
			VZT_ERROR("Failed to load {}", path.string());

			return {};
		}

		std::unordered_map<int, uint32_t> tinyObjToVazteranVertexIndices;
		std::unordered_map<int, uint32_t> tinyObjToVazteranMaterialIndices;

		Mesh result{};

		// First material is default one in case of shape "-1" material index
		result.materials.emplace_back();
		tinyObjToVazteranMaterialIndices[-1] = 0;
		for (int materialId = 0; materialId < materials.size(); materialId++)
		{
			tinyObjToVazteranMaterialIndices[materialId] = static_cast<uint32_t>(result.materials.size());
			auto currentMaterial                         = materials[materialId];

			if (currentMaterial.name == "None")
			{
				materials.emplace_back();
			}
			else
			{
				Image ambientTexture{};
				Image diffuseTexture{};
				Image specularTexture{};
				if (fs::exists(currentMaterial.ambient_texname))
					ambientTexture = vzt::Image(currentMaterial.ambient_texname);
				if (fs::exists(currentMaterial.diffuse_texname))
					diffuseTexture = vzt::Image(currentMaterial.diffuse_texname);
				if (fs::exists(currentMaterial.specular_texname))
					specularTexture = vzt::Image(currentMaterial.specular_texname);

				result.materials.emplace_back(Material{
				    diffuseTexture,
				    vzt::Vec4{currentMaterial.diffuse[0], currentMaterial.diffuse[1], currentMaterial.diffuse[2], 1.f},
				    currentMaterial.shininess});
			}
		}

		result.vertices.reserve(attributes.vertices.size());
		result.normals.reserve(attributes.vertices.size());
		for (const auto& shape : shapes)
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
						tinyObjToVazteranVertexIndices[index.vertex_index] =
						    static_cast<uint32_t>(result.vertices.size());
						vzt::Vec3 vertex{attributes.vertices[3 * static_cast<size_t>(index.vertex_index) + 0],
						                 attributes.vertices[3 * static_cast<size_t>(index.vertex_index) + 1],
						                 attributes.vertices[3 * static_cast<size_t>(index.vertex_index) + 2]};

						// Check if `normal_index` is zero or positive. negative = no normal data
						vzt::Vec3 normal{};
						if (index.normal_index >= 0)
						{
							normal.x = attributes.normals[3 * static_cast<size_t>(index.normal_index) + 0];
							normal.y = attributes.normals[3 * static_cast<size_t>(index.normal_index) + 1];
							normal.z = attributes.normals[3 * static_cast<size_t>(index.normal_index) + 2];
						}

						// Check if `texcoord_index` is zero or positive. negative = no texcoord data
						vzt::Vec2 texCoord{};
						if (index.texcoord_index >= 0)
						{
							texCoord.x = attributes.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 0];
							texCoord.y = attributes.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 1];
						}

						result.vertices.emplace_back(vertex);
						result.normals.emplace_back(normal);
						result.uvs.emplace_back(texCoord);
					}
					subMeshVertexIndices.push_back(tinyObjToVazteranVertexIndices[index.vertex_index]);
				}
				index_offset += fv;
			}

			// Handle only one subMesh material
			auto materialId = shape.mesh.material_ids[0];
			result.subMeshes.push_back({tinyObjToVazteranMaterialIndices[materialId], subMeshVertexIndices});
		}

		result.aabb = {result.vertices};
		return result;
	}
} // namespace vzt
