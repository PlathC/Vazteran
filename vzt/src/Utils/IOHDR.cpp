#include "vzt/Utils/IOHDR.hpp"

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

#include "vzt/Core/Logger.hpp"

namespace vzt
{
    Image<float> readEXR(const Path& path)
    {
        const std::string pathStr = path.string();
        if (!std::filesystem::exists(path))
        {
            vzt::logger::error("Can't find file at {}.", pathStr);
            return {};
        }

        float*      out; // width * height * RGBA
        int         width;
        int         height;
        const char* layerName = nullptr;
        const char* err       = nullptr;

        int          ret = LoadEXRWithLayer(&out, &width, &height, pathStr.c_str(), layerName, &err);
        Image<float> result{};
        if (ret != TINYEXR_SUCCESS)
        {
            if (err)
            {
                vzt::logger::error("{}", err);
                FreeEXRErrorMessage(err); // release memory of error message.
            }
        }
        else
        {
            result.width    = static_cast<uint32_t>(width);
            result.height   = static_cast<uint32_t>(height);
            result.channels = 4;

            result.data.resize(result.width * result.height * result.channels);
            std::memcpy(result.data.data(), out, result.width * result.height * result.channels * sizeof(float));
            free(out); // release memory of image data
        }

        return result;
    }
} // namespace vzt
