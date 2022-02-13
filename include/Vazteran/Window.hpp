#ifndef VAZTERAN_WINDOW_HPP
#define VAZTERAN_WINDOW_HPP

#include <functional>
#include <memory>

#define GLFW_INCLUDE_VULKAN

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Core/Utils.hpp"

#include "Vazteran/WindowTypes.hpp"

namespace vzt
{
	class Instance;
	struct SurfaceHandler
	{
	  public:
		SurfaceHandler(vzt::Instance* instance, VkSurfaceKHR surface);
		VkSurfaceKHR vkHandle() const { return m_surface; }
		~SurfaceHandler();

	  private:
		vzt::Instance* m_instance;
		VkSurfaceKHR   m_surface;
	};

	using OnFrameBufferChangedCallback = std::function<void()>;
	using OnKeyActionCallback =
	    std::function<void(vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers)>;
	using OnMousePosChangedCallback = std::function<void(const vzt::Dvec2 pos)>;
	using OnMouseButtonCallback =
	    std::function<void(vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers)>;

	class Window
	{
	  public:
		Window(std::string_view name, uint32_t width, uint32_t height, OnFrameBufferChangedCallback callback);
		~Window();

		void onFramebufferSizeChanged() const;
		void onKeyAction(vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers);
		void onMousePosChanged(const vzt::Dvec2 pos);
		void onMouseButton(vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers);

		void setOnKeyActionCallback(OnKeyActionCallback callback);
		void setOnMousePosChangedCallback(OnMousePosChangedCallback callback);
		void setOnMouseButtonCallback(OnMouseButtonCallback callback);

		GLFWwindow* windowHandle() const { return m_window.get(); }

		vzt::Size2D<uint32_t> getFrameBufferSize() const;
		VkSurfaceKHR          getSurface(vzt::Instance* instance);

		bool update();
		bool shouldClose() const { return glfwWindowShouldClose(m_window.get()); }

		std::vector<const char*> vkExtensions() const;

	  private:
		std::unique_ptr<SurfaceHandler> m_surface = nullptr;

		uint32_t m_width;
		uint32_t m_height;

		using GLFWwindowPtr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;
		GLFWwindowPtr m_window;

		OnFrameBufferChangedCallback m_onFrameBufferChangedCallback;
		OnKeyActionCallback          m_onKeyActionCallback;
		OnMousePosChangedCallback    m_onMousePosChangedCallback;
		OnMouseButtonCallback        m_onMouseButtonCallback;

		vzt::Dvec2 m_lastMousePos;
	};
} // namespace vzt

#endif // VAZTERAN_WINDOW_HPP
