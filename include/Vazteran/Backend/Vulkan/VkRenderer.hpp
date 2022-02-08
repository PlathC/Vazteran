#ifndef VAZTERAN_BACKEND_VULKAN_RENDERER_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Backend/Vulkan/SwapChain.hpp"
#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Camera.hpp"

struct GLFWwindow;

namespace vzt
{
	class GraphicPipeline;
	class ImageView;
	class MeshView;
	class RenderPass;
	class Scene;
	class VkUiRenderer;

	class Renderer
	{
	  public:
		Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size);

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		Renderer(Renderer&&);
		Renderer& operator=(Renderer&&);

		~Renderer();

		void               SetScene(vzt::Scene* scene);
		void               Draw(const vzt::Camera& camera);
		const vzt::Device* Device() const { return &m_device; }
		void               FrameBufferResized(vzt::Size2D<uint32_t> newSize);

	  private:
		void BuildRenderingSupports();

	  private:
		VkSurfaceKHR m_surface;

		vzt::Device    m_device;
		vzt::SwapChain m_swapChain;

		std::unique_ptr<vzt::MeshView>     m_meshView;
		std::unique_ptr<vzt::VkUiRenderer> m_ui;

		std::vector<VkSemaphore>      m_offscreenSemaphores{};
		std::vector<vzt::FrameBuffer> m_offscreenFrames;
		VkDescriptorSetLayout         m_offscreenDescriptorLayout;
		vzt::DescriptorPool           m_offscreenDescriptors;
		std::vector<vzt::FrameBuffer> m_frames;

		vzt::CommandPool m_deferredCommandPool;

		VkDescriptorSetLayout                  m_fsDescriptorSetLayout;
		std::unique_ptr<vzt::GraphicPipeline>  m_compositionPipeline;
		std::vector<vzt::SamplerDescriptorSet> m_samplersDescriptors;
		vzt::DescriptorPool                    m_deferredDescriptorPool;

		vzt::Instance* m_instance;
		GLFWwindow*    m_window;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERER_HPP
