#ifndef VAZTERAN_LIGHT_HPP
#define VAZTERAN_LIGHT_HPP

#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct DirectionalLight
	{
		Vec4 direction;
		Vec4 color = Vec4{1.f};
	};
} // namespace vzt

#endif // VAZTERAN_LIGHT_HPP
