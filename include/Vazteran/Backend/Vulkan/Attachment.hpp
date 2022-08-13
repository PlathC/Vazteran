#ifndef VAZTERAN_VULKAN_ATTACHMENT_HPP
#define VAZTERAN_VULKAN_ATTACHMENT_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	class Device;

	enum class LoadOperation : uint32_t
	{
		Load     = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD,
		Clear    = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
		DontCare = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE
	};
	TO_VULKAN_FUNCTION(LoadOperation, VkAttachmentLoadOp)

	enum class StoreOperation : uint32_t
	{
		Store    = VK_ATTACHMENT_STORE_OP_STORE,
		DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		// NoneKHR  = VK_ATTACHMENT_STORE_OP_NONE_KHR,
		NoneQCOM = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
		NoneExt  = VK_ATTACHMENT_STORE_OP_NONE_EXT
	};
	TO_VULKAN_FUNCTION(StoreOperation, VkAttachmentStoreOp)

	class Attachment
	{
	  public:
		Attachment(const Device* device, Uvec2 size, Format format, ImageUsage usage);
		Attachment(const Device* device, VkImage image, Format format, ImageLayout layout, ImageAspect aspect);
		~Attachment();

		Attachment(const Attachment&)            = delete;
		Attachment& operator=(const Attachment&) = delete;

		Attachment(Attachment&&)            = default;
		Attachment& operator=(Attachment&&) = default;

		const ImageView* getView() const { return m_imageView.get(); }
		Format           getFormat() const { return m_format; }
		ImageLayout      getLayout() const { return m_layout; }
		SampleCount      getSampleCount() const { return m_sampleCount; }
		void             setSampleCount(const SampleCount sampleCount) { m_sampleCount = sampleCount; }

		Texture* asTexture() const;

	  private:
		const Device* m_device;
		Format        m_format;
		ImageLayout   m_layout;
		SampleCount   m_sampleCount;

		std::unique_ptr<ImageView> m_imageView;
		std::unique_ptr<Texture>   m_textureRepresentation;
	};
} // namespace vzt

#endif // VAZTERAN_VULKAN_ATTACHMENT_HPP
