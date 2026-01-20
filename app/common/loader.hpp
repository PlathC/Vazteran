#ifndef VZT_COMMON_LOADER_HPP
#define VZT_COMMON_LOADER_HPP

#include "vzt/core/file.hpp"
#include "vzt/core/math.hpp"
#include "vzt/core/type.hpp"

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

    Mesh readObj(const Path& path);
} // namespace vzt

#endif // VZT_COMMON_LOADER_HPP