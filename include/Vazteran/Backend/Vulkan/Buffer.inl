#include "Vazteran/Backend/Vulkan/Device.hpp"

namespace vzt
{
	template <class Type>
	Buffer::Buffer(const vzt::Device* device, const std::vector<Type>& data, BufferUsage usage, MemoryUsage memoryUsage,
	               bool mappable, bool persistent)
	    : Buffer(device, sizeof(Type) * data.size(), reinterpret_cast<uint8_t const*>(data.data()), usage, memoryUsage,
	             mappable, persistent)
	{
	}

	template <class Type>
	void Buffer::update(const std::vector<Type>& newData)
	{
		update(sizeof(Type) * newData.size(), reinterpret_cast<uint8_t const*>(newData.data()));
	}

} // namespace vzt
