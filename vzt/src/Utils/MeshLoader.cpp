#include "vzt/Utils/MeshLoader.hpp"

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "vzt/Core/Logger.hpp"

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

            logger::warn(warnings);
            logger::error(errors);
            logger::error("Failed to load {}", path.string());

            return {};
        }

        std::unordered_map<int, uint32_t> tinyObjToVazteranVertexIndices;

        Mesh result{};

        result.vertices.reserve(attributes.vertices.size());
        result.normals.reserve(attributes.vertices.size());
        result.indices.reserve(attributes.vertices.size() * 3);

        std::size_t indexOffset = 0;
        for (const auto& shape : shapes)
        {
            std::size_t tinyObjIndexOffset = 0;

            // Loop over vertices in the face.
            std::vector<uint32_t> subMeshVertexIndices;
            subMeshVertexIndices.reserve(shape.mesh.indices.size());
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                std::size_t fv = shape.mesh.num_face_vertices[f];
                for (std::size_t vertexIndex = 0; vertexIndex < fv; vertexIndex++)
                {
                    tinyobj::index_t index = shape.mesh.indices[tinyObjIndexOffset + vertexIndex];
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
                    }
                    result.indices.emplace_back(tinyObjToVazteranVertexIndices[index.vertex_index]);
                }
                tinyObjIndexOffset += fv;
            }

            // Handle only one subMesh material

            Range<> range{indexOffset, result.indices.size()};
            assert(shape.mesh.indices.size() == range.size());

            result.subMeshes.emplace_back(SubMesh{range});
            indexOffset += result.indices.size();
        }

        return result;
    }

} // namespace vzt
