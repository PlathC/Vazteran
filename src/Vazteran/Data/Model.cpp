#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vazteran/Data/Model.hpp"
#include <iostream>

namespace vzt {
    Model::Model(const fs::path& modelPath){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string errorMessage;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &errorMessage, modelPath.string().c_str(),
                              (modelPath.parent_path().string() + "/").c_str())) {
            throw std::runtime_error(errorMessage);
        }

        // TODO: Handle more than one material per Model
        if (!materials.empty()){
            m_material = {
                    Image({materials[0].ambient[0], materials[0].ambient[1], materials[0].ambient[2], 1.f}),
                    Image({materials[0].diffuse[0], materials[0].diffuse[1], materials[0].diffuse[2], 1.f}),
                    Image({materials[0].specular[0], materials[0].specular[1], materials[0].specular[2], 1.f})
            };
        }else {
            m_material = {
                    Image({1.f, 1.f, 1.f, 1.f}),
                    Image({1.f, 1.f, 1.f, 1.f}),
                    Image({1.f, 1.f, 1.f, 1.f}),
            };
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                vzt::Vertex vertex{};
                vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.textureCoordinates = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    if (uniqueVertices.empty())
                        m_aabb.minimum = m_aabb.maximum = vertex.position;

                    uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                    m_vertices.push_back(vertex);

                    if (vertex.position.x < m_aabb.minimum.x) {
                        m_aabb.minimum.x = vertex.position.x;
                    }

                    if (vertex.position.y < m_aabb.minimum.y) {
                        m_aabb.minimum.y = vertex.position.y;
                    }

                    if (vertex.position.z < m_aabb.minimum.z) {
                        m_aabb.minimum.z = vertex.position.z;
                    }

                    if (vertex.position.x > m_aabb.maximum.x) {
                        m_aabb.maximum.x = vertex.position.x;
                    }

                    if (vertex.position.y > m_aabb.maximum.y) {
                        m_aabb.maximum.y = vertex.position.y;
                    }

                    if (vertex.position.z > m_aabb.maximum.z) {
                        m_aabb.maximum.z = vertex.position.z;
                    }
                }

                m_indices.push_back(uniqueVertices[vertex]);
            }
        }

        m_aabb.vertices = std::array<glm::vec3, 8>{
                // Bottom
                glm::vec3{ m_aabb.minimum.x, m_aabb.minimum.y, m_aabb.minimum.z },
                glm::vec3{ m_aabb.maximum.x, m_aabb.minimum.y, m_aabb.minimum.z },
                glm::vec3{ m_aabb.minimum.x, m_aabb.maximum.y, m_aabb.minimum.z },
                glm::vec3{ m_aabb.maximum.x, m_aabb.maximum.y, m_aabb.minimum.z },

                // Top
                glm::vec3{ m_aabb.minimum.x, m_aabb.minimum.y, m_aabb.maximum.z },
                glm::vec3{ m_aabb.maximum.x, m_aabb.minimum.y, m_aabb.maximum.z },
                glm::vec3{ m_aabb.minimum.x, m_aabb.maximum.y, m_aabb.maximum.z },
                glm::vec3{ m_aabb.maximum.x, m_aabb.maximum.y, m_aabb.maximum.z },
        };
    }

    AABB Model::BoundingBox() const {
        // https://stackoverflow.com/a/58630206
        auto movedAABB = m_aabb;

        glm::mat4 translated           = glm::translate(glm::mat4(1.f), {0.f, 0.f, 0.f});
        glm::mat4 translatedRotateX    = glm::rotate(translated, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
        glm::mat4 translatedRotateXY   = glm::rotate(translatedRotateX, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 transformationMatrix = glm::rotate(translatedRotateXY, m_rotation.z, glm::vec3(0., 0., 1.));

        for (auto& vertex : movedAABB.vertices) {
            vertex = (transformationMatrix * glm::vec4(vertex, 1.f));

            if (vertex.x < movedAABB.minimum.x) {
                movedAABB.minimum.x = vertex.x;
            }

            if (vertex.y < movedAABB.minimum.y) {
                movedAABB.minimum.y = vertex.y;
            }

            if (vertex.z < movedAABB.minimum.z) {
                movedAABB.minimum.z = vertex.z;
            }

            if (vertex.x > movedAABB.maximum.x) {
                movedAABB.maximum.x = vertex.x;
            }

            if (vertex.y > movedAABB.maximum.y) {
                movedAABB.maximum.y = vertex.y;
            }

            if (vertex.z > movedAABB.maximum.z) {
                movedAABB.maximum.z = vertex.z;
            }
        }

        movedAABB.vertices = std::array<glm::vec3, 8>{
                // Bottom
                glm::vec3{ movedAABB.minimum.x, movedAABB.minimum.y, movedAABB.minimum.z },
                glm::vec3{ movedAABB.maximum.x, movedAABB.minimum.y, movedAABB.minimum.z },
                glm::vec3{ movedAABB.minimum.x, movedAABB.maximum.y, movedAABB.minimum.z },
                glm::vec3{ movedAABB.maximum.x, movedAABB.maximum.y, movedAABB.minimum.z },

                // Top
                glm::vec3{ movedAABB.minimum.x,movedAABB.minimum.y, movedAABB.maximum.z },
                glm::vec3{ movedAABB.maximum.x,movedAABB.minimum.y, movedAABB.maximum.z },
                glm::vec3{ movedAABB.minimum.x,movedAABB.maximum.y, movedAABB.maximum.z },
                glm::vec3{ movedAABB.maximum.x,movedAABB.maximum.y, movedAABB.maximum.z },
        };

        return movedAABB;
    }

    glm::mat4 Model::ModelMatrix() const {
        auto translated = glm::translate(glm::mat4(1.0), m_position);
        auto translatedRotateX = glm::rotate(translated, m_rotation.x, glm::vec3(1., 0., 0.));
        auto translatedRotateXY = glm::rotate(translatedRotateX, m_rotation.y, glm::vec3(0., 1., 0.));
        return glm::rotate(translatedRotateXY, m_rotation.z, glm::vec3(0., 0., 1.));
    }
}