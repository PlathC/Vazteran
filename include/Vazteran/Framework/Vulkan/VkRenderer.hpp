#ifndef VAZTERAN_RENDERER_HPP
#define VAZTERAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Framework/Vulkan/Buffer.hpp"
#include "Vazteran/Framework/Vulkan/CommandPool.hpp"
#include "Vazteran/Framework/Vulkan/Instance.hpp"
#include "Vazteran/Framework/Vulkan/RenderObject.hpp"
#include "Vazteran/Framework/Vulkan/SwapChain.hpp"

struct GLFWwindow;

namespace vzt
{
	class GraphicPipeline;
	class ImageView;
	class LogicalDevice;
	class Model;
	class PhysicalDevice;
	class RenderPass;
	class UiRenderer;

	class Renderer
	{
	  public:
		Renderer(
		    vzt::Instance *instance, GLFWwindow *window, VkSurfaceKHR surface, vzt::Size2D<int> size,
		    vzt::Camera camera, std::vector<vzt::Model *> models = {});

		void Draw();
		vzt::LogicalDevice *Device() const
		{
			return m_logicalDevice.get();
		}
		RenderPass *FinalPass() const;
		void FrameBufferResized(vzt::Size2D<int> newSize);

	  private:
		VkSurfaceKHR m_surface;

		std::unique_ptr<vzt::PhysicalDevice> m_physicalDevice;
		std::unique_ptr<vzt::LogicalDevice> m_logicalDevice;
		std::unique_ptr<vzt::SwapChain> m_swapChain;
		std::unique_ptr<vzt::UiRenderer> m_uiRenderer;

		std::vector<std::unique_ptr<vzt::RenderObject>> m_objects;
		vzt::Camera m_camera;
	};
} // namespace vzt

#endif // VAZTERAN_RENDERER_HPP
