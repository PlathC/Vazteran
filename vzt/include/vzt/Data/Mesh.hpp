#ifndef VZT_DATA_MESH_HPP
#define VZT_DATA_MESH_HPP

#include <vector>

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Type.hpp"

namespace vzt
{
    struct SubMesh
    {
        Range<> indices;
    };

    struct Mesh
    {
        std::vector<SubMesh>  subMeshes;
        std::vector<Vec3>     vertices;
        std::vector<uint32_t> indices;
        std::vector<Vec3>     normals;
    };
} // namespace vzt

#endif // VZT_DATA_MESH_HPP
