#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Data/Image.hpp"

namespace vzt
{
	struct Material
	{
		vzt::Image ambientMap  = vzt::Image({1.f, 1.f, 1.f, 1.f});
		vzt::Image diffuseMap  = vzt::Image({1.f, 1.f, 1.f, 1.f});
		vzt::Image specularMap = vzt::Image({1.f, 1.f, 1.f, 1.f});

		vzt::Vec4 ambientColor  = {0.05f, 0.2f, 0.05f, 1.};
		vzt::Vec4 diffuseColor  = {0.3f, 0.5f, 0.3f, 1.};
		vzt::Vec4 specularColor = {0.6f, 0.6f, 0.6f, 1.};
		float     shininess     = 50.;

		static const vzt::Material Default;
	};
} // namespace vzt

#endif // VAZTERAN_MATERIAL_HPP
