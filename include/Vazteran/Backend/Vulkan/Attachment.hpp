#ifndef VAZTERAN_BACKEND_VULKAN_ATTACHMENT_HPP
#define VAZTERAN_BACKEND_VULKAN_ATTACHMENT_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Core/Math.hpp"

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
		Attachment(const vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage);
		Attachment(const vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageLayout layout,
		           vzt::ImageAspect aspect);
		~Attachment();

		Attachment(const Attachment&) = delete;
		Attachment& operator=(const Attachment&) = delete;

		Attachment(Attachment&&) = default;
		Attachment& operator=(Attachment&&) = default;

		const vzt::ImageView* getView() const { return m_imageView.get(); }
		vzt::Format           getFormat() const { return m_format; }
		vzt::ImageLayout      getLayout() const { return m_layout; }
		vzt::SampleCount      getSampleCount() const { return m_sampleCount; }
		void                  setSampleCount(const vzt::SampleCount sampleCount) { m_sampleCount = sampleCount; }

		vzt::Texture* asTexture() const;

	  private:
		const vzt::Device*              m_device;
		vzt::Format                     m_format;
		vzt::ImageLayout                m_layout;
		vzt::SampleCount                m_sampleCount;
		std::unique_ptr<vzt::ImageView> m_imageView;

		std::unique_ptr<vzt::Texture> m_textureRepresentation;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_ATTACHMENT_HPP
