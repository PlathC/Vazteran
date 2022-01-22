#ifndef VAZTERAN_WINDOW_HPP
#define VAZTERAN_WINDOW_HPP

#include <functional>
#include <memory>

#define GLFW_INCLUDE_VULKAN

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Core/Utils.hpp"

#include "Vazteran/WindowType.hpp"

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

		void OnFramebufferSizeChanged() const;
		void OnKeyAction(vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers);
		void OnMousePosChanged(const vzt::Dvec2 pos);
		void OnMouseButton(vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers);

		void SetOnKeyActionCallback(OnKeyActionCallback callback);
		void SetOnMousePosChangedCallback(OnMousePosChangedCallback callback);
		void SetOnMouseButtonCallback(OnMouseButtonCallback callback);

		vzt::Size2D<uint32_t>    FrameBufferSize() const;
		GLFWwindow*              Handle() const { return m_window.get(); }
		bool                     Update();
		bool                     ShouldClose() const { return glfwWindowShouldClose(m_window.get()); }
		VkSurfaceKHR             Surface(vzt::Instance* instance);
		std::vector<const char*> VkExtensions() const;

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
