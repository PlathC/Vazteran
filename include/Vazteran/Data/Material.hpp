#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Data/Image.hpp"

namespace vzt
{
	struct Material
	{
		std::optional<vzt::Image> texture   = vzt::Image(vzt::Vec4{1.f});
		vzt::Vec4                 color     = vzt::Vec4{.7f};
		float                     shininess = 50.f;

		static const vzt::Material Default;
	};
} // namespace vzt

#endif // VAZTERAN_MATERIAL_HPP
