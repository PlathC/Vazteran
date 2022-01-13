#ifndef VAZTERAN_MATERIAL_HPP
#define VAZTERAN_MATERIAL_HPP

#include <optional>

#include <glm/glm.hpp>

#include "Vazteran/Data/Image.hpp"
#include "Vazteran/Data/Types.hpp"

namespace vzt
{
	struct Material
	{
		vzt::Image ambientMap = vzt::Image({1.f, 1.f, 1.f, 1.f});
		vzt::Image diffuseMap = vzt::Image({1.f, 1.f, 1.f, 1.f});
		vzt::Image specularMap = vzt::Image({1.f, 1.f, 1.f, 1.f});
		;

		vzt::Color ambientColor = vzt::Color{0.05f, 0.2f, 0.05f, 1.};
		vzt::Color diffuseColor = vzt::Color{0.3f, 0.5f, 0.3f, 1.};
		vzt::Color specularColor = vzt::Color{0.6f, 0.6f, 0.6f, 1.};
		float shininess = 50.;

		static const vzt::Material Default;
	};
} // namespace vzt

#endif // VAZTERAN_MATERIAL_HPP
