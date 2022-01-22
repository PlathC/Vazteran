#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Data/Image.hpp"

namespace vzt
{
	struct Material
	{
		vzt::Image ambientMap  = vzt::Image(vzt::Vec4{1.f});
		vzt::Image diffuseMap  = vzt::Image(vzt::Vec4{1.f});
		vzt::Image specularMap = vzt::Image(vzt::Vec4{1.f});

		vzt::Vec4 ambientColor  = vzt::Vec4{.7f};
		vzt::Vec4 diffuseColor  = vzt::Vec4{.7f};
		vzt::Vec4 specularColor = vzt::Vec4{.7f};
		float     shininess     = 50.f;

		static const vzt::Material Default;
	};
} // namespace vzt

#endif // VAZTERAN_MATERIAL_HPP
