#ifndef VAZTERAN_FRAMEWORK_VULKAN_RENDERER_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Framework/Vulkan/Buffer.hpp"
#include "Vazteran/Framework/Vulkan/CommandPool.hpp"
#include "Vazteran/Framework/Vulkan/Instance.hpp"
#include "Vazteran/Framework/Vulkan/SwapChain.hpp"

struct GLFWwindow;

namespace vzt
{
	class GraphicPipeline;
	class ImageView;
	class Device;
	class MeshView;
	class RenderPass;
	class Scene;

	class Renderer
	{
	  public:
		Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size);
		~Renderer();

		void         SetScene(vzt::Scene* scene);
		void         Draw(const vzt::Camera& camera);
		vzt::Device* Device() const { return m_device.get(); }
		void         FrameBufferResized(vzt::Size2D<uint32_t> newSize);

	  private:
		VkSurfaceKHR m_surface;

		std::unique_ptr<vzt::Device>    m_device;
		std::unique_ptr<vzt::SwapChain> m_swapChain;
		std::unique_ptr<vzt::MeshView>  m_meshView;
	};
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_RENDERER_HPP
