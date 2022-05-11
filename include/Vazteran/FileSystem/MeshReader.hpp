#ifndef VAZTERAN_LOADER_HPP
#define VAZTERAN_LOADER_HPP

#include "Vazteran/Data/Mesh.hpp"
#include "Vazteran/FileSystem/File.hpp"

namespace vzt
{
	Mesh readObj(const Path& path);
}

#endif // VAZTERAN_LOADER_HPP
