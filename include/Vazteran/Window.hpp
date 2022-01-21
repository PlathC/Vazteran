#ifndef VAZTERAN_WINDOW_HPP
#define VAZTERAN_WINDOW_HPP

#include <functional>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	class Instance;
	struct SurfaceHandler
	{
	  public:
		SurfaceHandler(vzt::Instance* instance, VkSurfaceKHR surface);
		VkSurfaceKHR VkHandle() const { return m_surface; }
		~SurfaceHandler();

	  private:
		vzt::Instance* m_instance;
		VkSurfaceKHR   m_surface;
	};

	using FrameBufferResizedCallback = std::function<void()>;

	class Window
	{
	  public:
		Window(std::string_view name, uint32_t width, uint32_t height, FrameBufferResizedCallback callback);

		void                     FrameBufferResized() const;
		vzt::Size2D<uint32_t>    FrameBufferSize() const;
		GLFWwindow*              Handle() const { return m_window.get(); }
		bool                     Update();
		bool                     ShouldClose() const { return glfwWindowShouldClose(m_window.get()); }
		VkSurfaceKHR             Surface(vzt::Instance* instance);
		std::vector<const char*> VkExtensions() const;

		~Window();

	  private:
		std::unique_ptr<SurfaceHandler> m_surface = nullptr;

		uint32_t m_width;
		uint32_t m_height;

		using GLFWwindowPtr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;
		GLFWwindowPtr              m_window;
		FrameBufferResizedCallback m_fbResizedCallback;

		vzt::Dvec2 m_mouseDelta;
		vzt::Dvec2 m_mousePos;
	};
} // namespace vzt

#endif // VAZTERAN_WINDOW_HPP
