#ifndef VAZTERAN_SYSTEM_HPP
#define VAZTERAN_SYSTEM_HPP

#include <entt/entity/handle.hpp>

namespace vzt
{
	using Entity = entt::handle;

	enum class SystemEvent
	{
		Construct,
		Update,
		Destroy
	};

} // namespace vzt

#endif // VAZTERAN_SYSTEM_HPP
