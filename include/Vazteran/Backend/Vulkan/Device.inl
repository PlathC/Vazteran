#include "Vazteran/Backend/Vulkan/Device.hpp"

namespace vzt
{
	template <class Type>
	uint64_t Device::computeUniformOffsetAlignment() const
	{
		const uint64_t minOffset = getMinUniformOffsetAlignment();
		uint64_t       typeSize  = sizeof(Type);
		if (minOffset > 0)
			typeSize = (typeSize + minOffset - 1) & ~(minOffset - 1);

		return typeSize;
	}
} // namespace vzt
