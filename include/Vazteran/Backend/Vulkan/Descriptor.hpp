#ifndef VAZTERAN_BACKEND_VULKAN_DESCRIPTOR_HPP
#define VAZTERAN_BACKEND_VULKAN_DESCRIPTOR_HPP

#include <stack>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"

namespace vzt
{
	class Attachment;
	class Device;

	template <class Type>
	using IndexedUniform = std::unordered_map<uint32_t, Type>;

	struct BufferDescriptor
	{
		uint32_t     offset;
		uint32_t     range;
		vzt::Buffer* buffer;
	};

	enum class DescriptorType
	{
		Sampler              = VK_DESCRIPTOR_TYPE_SAMPLER,
		CombinedSampler      = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		SampledImage         = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		StorageImage         = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		UniformTexelBuffer   = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
		StorageTexelBuffer   = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
		UniformBuffer        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		StorageBuffer        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		UniformBufferDynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		InputAttachment      = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	};

	class DescriptorPool
	{
	  public:
		DescriptorPool() = default;
		DescriptorPool(Device* device, const std::vector<DescriptorType> descriptorTypes, uint32_t maxSetNb = 64,
		               VkDescriptorPoolCreateFlags flags = 0);

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;

		DescriptorPool(DescriptorPool&&) noexcept;
		DescriptorPool& operator=(DescriptorPool&&) noexcept;

		~DescriptorPool();

		void Allocate(uint32_t count, VkDescriptorSetLayout layout);
		void Bind(uint32_t i, VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
		          VkPipelineLayout pipelineLayout) const;

		void Update(const std::size_t i, const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors,
		            const IndexedUniform<vzt::Texture*>& imageDescriptors);
		void Update(const std::size_t i, const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors);
		void Update(const std::size_t i, const IndexedUniform<vzt::Texture*>& imageDescriptors);

		void UpdateAll(const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors,
		               const IndexedUniform<vzt::Texture*>&         imageDescriptors);
		void UpdateAll(const IndexedUniform<vzt::BufferDescriptor>& bufferDescriptors);
		void UpdateAll(const IndexedUniform<vzt::Texture*>& imageDescriptors);

		uint32_t         Remaining() const { return static_cast<uint32_t>(m_maxSetNb - m_descriptors.size()); }
		uint32_t         MaxSetNb() const { return m_maxSetNb; }
		VkDescriptorPool VkHandle() const { return m_vkHandle; }

	  private:
		vzt::Device*                 m_device   = nullptr;
		VkDescriptorPool             m_vkHandle = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_descriptors;

		uint32_t m_maxSetNb = 0;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_DESCRIPTOR_HPP
