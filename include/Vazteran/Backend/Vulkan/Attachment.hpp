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

	enum class StoreOperation : uint32_t
	{
		Store    = VK_ATTACHMENT_STORE_OP_STORE,
		DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		NoneKHR  = VK_ATTACHMENT_STORE_OP_NONE_KHR,
		NoneQCOM = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
		NoneExt  = VK_ATTACHMENT_STORE_OP_NONE_EXT
	};

	class Attachment
	{
	  public:
		Attachment(vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage);
		Attachment(vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageLayout layout,
		           vzt::ImageAspect aspect);
		~Attachment() = default;

		Attachment(const Attachment&) = delete;
		Attachment& operator=(const Attachment&) = delete;

		Attachment(Attachment&&) = default;
		Attachment& operator=(Attachment&&) = default;

		const vzt::ImageView*  getView() const { return m_imageView.get(); }
		const vzt::Format      getFormat() const { return m_format; }
		const vzt::ImageLayout getLayout() const { return m_layout; }

		vzt::Texture* asTexture();

		void setLoadOperation(vzt::LoadOperation loadOp) { m_loadOp = loadOp; }
		void setStoreOperation(vzt::StoreOperation storeOp) { m_storeOp = storeOp; }

		void setStencilLoadOperation(vzt::LoadOperation loadOp) { m_stencilLoadOp = loadOp; }
		void setStencilStoreOperation(vzt::StoreOperation storeOp) { m_stencilStoreOp = storeOp; }

		void setInitialLayout(vzt::ImageLayout initialLayout) { m_initialLayout = initialLayout; }
		void setFinalLayout(vzt::ImageLayout finalLayout) { m_finalLayout = finalLayout; }

		VkAttachmentDescription getDescription() const;

	  private:
		vzt::Device*                    m_device;
		vzt::Format                     m_format;
		vzt::ImageLayout                m_layout;
		std::unique_ptr<vzt::ImageView> m_imageView;

		vzt::LoadOperation  m_loadOp  = LoadOperation::Clear;
		vzt::StoreOperation m_storeOp = StoreOperation::Store;

		vzt::LoadOperation  m_stencilLoadOp  = LoadOperation::DontCare;
		vzt::StoreOperation m_stencilStoreOp = StoreOperation::DontCare;

		vzt::ImageLayout m_initialLayout = vzt::ImageLayout::Undefined;
		vzt::ImageLayout m_finalLayout   = vzt::ImageLayout::PresentSrcKHR;

		std::unique_ptr<vzt::Texture> m_textureRepresentation;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_ATTACHMENT_HPP
