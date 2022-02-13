#include "Vazteran/Backend/Vulkan/GpuObjects.hpp"

namespace vzt
{
	GenericMaterial GenericMaterial::fromMaterial(const vzt::Material& original)
	{
		return GenericMaterial{glm::vec4(glm::vec3(original.color), original.shininess)};
	}
} // namespace vzt
