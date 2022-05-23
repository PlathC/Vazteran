#ifndef VAZTERAN_VULKAN_DESCRIPTOR_HPP
#define VAZTERAN_VULKAN_DESCRIPTOR_HPP

#include <stack>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/Shader.hpp"

namespace vzt
{
	class Attachment;

	template <class Type>
	using IndexedUniform = std::unordered_map<uint32_t, Type>;

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
	TO_VULKAN_FUNCTION(DescriptorType, VkDescriptorType)

	class DescriptorLayout
	{
	  public:
		DescriptorLayout();

		DescriptorLayout(const DescriptorLayout& other);
		DescriptorLayout& operator=(const DescriptorLayout& other);

		DescriptorLayout(DescriptorLayout&& other) noexcept;
		DescriptorLayout& operator=(DescriptorLayout&& other) noexcept;

		~DescriptorLayout();

		void addBinding(const uint32_t binding, const vzt::DescriptorType type);
		void configure(const vzt::Device* const device);

		const VkDescriptorSetLayout& vkHandle() const;

	  private:
		const vzt::Device*            m_device = nullptr;
		mutable VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;

		using Binding = std::tuple<uint32_t /*binding*/, vzt::DescriptorType /*type */>;
		std::vector<Binding> m_bindings;
	};

	class DescriptorPool
	{
	  public:
		DescriptorPool() = default;
		DescriptorPool(const vzt::Device* const device, const std::vector<DescriptorType> descriptorTypes,
		               uint32_t maxSetNb = 64, VkDescriptorPoolCreateFlags flags = 0);

		DescriptorPool(const DescriptorPool&)            = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;

		DescriptorPool(DescriptorPool&&) noexcept;
		DescriptorPool& operator=(DescriptorPool&&) noexcept;

		~DescriptorPool();

		void allocate(uint32_t count, const vzt::DescriptorLayout& layout);
		void bind(uint32_t i, VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
		          VkPipelineLayout pipelineLayout) const;

		VkDescriptorSet operator[](uint32_t i) const { return m_descriptors[i]; }

		void update(const std::size_t i, const IndexedUniform<BufferSpan>& bufferDescriptors,
		            const IndexedUniform<vzt::Texture*>& imageDescriptors);
		void update(const std::size_t i, const IndexedUniform<BufferSpan>& bufferDescriptors);
		void update(const std::size_t i, const IndexedUniform<Texture*>& imageDescriptors);

		void updateAll(const IndexedUniform<BufferSpan>& bufferDescriptors,
		               const IndexedUniform<Texture*>&   imageDescriptors);
		void updateAll(const IndexedUniform<BufferSpan>& bufferDescriptors);
		void updateAll(const IndexedUniform<Texture*>& imageDescriptors);

		uint32_t         getRemaining() const { return static_cast<uint32_t>(m_maxSetNb - m_descriptors.size()); }
		uint32_t         getMaxSetNb() const { return m_maxSetNb; }
		VkDescriptorPool vkHandle() const { return m_vkHandle; }

	  private:
		const vzt::Device*           m_device   = nullptr;
		VkDescriptorPool             m_vkHandle = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_descriptors;

		uint32_t m_maxSetNb = 0;
	};
} // namespace vzt

#endif // VAZTERAN_VULKAN_DESCRIPTOR_HPP
