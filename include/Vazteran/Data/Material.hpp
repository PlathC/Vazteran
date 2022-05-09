#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include "Vazteran/Data/Image.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct Material
	{
		std::optional<vzt::Image> texture   = vzt::Image(vzt::Vec4{1.f});
		vzt::Vec4                 color     = vzt::Vec4{.7f};
		float                     shininess = 50.f;

		static const vzt::Material default;
	};
} // namespace vzt

#endif // VAZTERAN_MATERIAL_HPP
