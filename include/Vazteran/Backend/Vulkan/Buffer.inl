#include "Vazteran/Backend/Vulkan/Device.hpp"

namespace vzt
{
	template <class Type>
	Buffer::Buffer(vzt::Device* device, const std::vector<Type>& data, VkBufferUsageFlags usage,
	               MemoryUsage memoryUsage)
	    : m_device(device)
	{
		create(sizeof(Type) * data.size(), reinterpret_cast<uint8_t const*>(data.data()), usage, memoryUsage);
	}

	template <class Type>
	void Buffer::update(const std::vector<Type>& newData)
	{
		update(sizeof(Type) * newData.size(), reinterpret_cast<uint8_t const*>(newData.data()));
	}

} // namespace vzt
