#include "Vazteran/Backend/Vulkan/Device.hpp"

namespace vzt
{
	template <class Type>
	Type BufferSpan::get(std::size_t i)
	{
		assert(parent.m_mappable && "The buffer must be signaled as mappable before being mapped to CPU data");

		Type* data = nullptr;
		Type  res;
		vmaMapMemory(parent.m_device->getAllocatorHandle(), parent.m_allocation, reinterpret_cast<void**>(&data));
		std::memcpy(&res, data + (i * sizeof(Type)), sizeof(Type));
		vmaUnmapMemory(parent.m_device->getAllocatorHandle(), parent.m_allocation);

		return res;
	}

	template <class Type>
	void BufferSpan::set(std::size_t i, const Type& type)
	{
		assert(parent.m_mappable && "The buffer must be signaled as mappable before being mapped to CPU data");

		Type* data = nullptr;
		vmaMapMemory(parent.m_device->getAllocatorHandle(), parent.m_allocation, reinterpret_cast<void**>(&data));
		std::memcpy(data + (i * sizeof(Type)), &type, sizeof(Type));
		vmaUnmapMemory(parent.m_device->getAllocatorHandle(), parent.m_allocation);

		return res;
	}

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
