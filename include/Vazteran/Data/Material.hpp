#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include "Vazteran/Core/Type.hpp"
#include "Vazteran/Data/Image.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct Material
	{
		Optional<Image> texture   = {};
		Vec4            color     = vzt::Vec4{.7f};
		float           shininess = 50.f;

		static const vzt::Material default;
	};
} // namespace vzt

#endif // VAZTERAN_MATERIAL_HPP
