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
		SurfaceHandler() = default;
		SurfaceHandler(vzt::Instance* instance, VkSurfaceKHR surface);

		SurfaceHandler(const SurfaceHandler&)            = delete;
		SurfaceHandler& operator=(const SurfaceHandler&) = delete;
		SurfaceHandler(SurfaceHandler&&);
		SurfaceHandler& operator=(SurfaceHandler&&);

		VkSurfaceKHR vkHandle() const { return m_surface; }
		~SurfaceHandler();

	  private:
		vzt::Instance* m_instance = nullptr;
		VkSurfaceKHR   m_surface  = VK_NULL_HANDLE;
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
		Window(std::string_view name, uint32_t width, uint32_t height);
		~Window();

		void onFramebufferSizeChanged() const;
		void onKeyAction(vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers);
		void onMousePosChanged(const vzt::Dvec2 pos);
		void onMouseButton(vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers);

		void setOnFrameBufferChangedCallback(OnFrameBufferChangedCallback callback);
		void setOnKeyActionCallback(OnKeyActionCallback callback);
		void setOnMousePosChangedCallback(OnMousePosChangedCallback callback);
		void setOnMouseButtonCallback(OnMouseButtonCallback callback);

		GLFWwindow*           getWindowHandle() const { return m_window.get(); }
		vzt::Size2D<uint32_t> getFrameBufferSize() const;
		VkSurfaceKHR          getSurface() const { return m_surface.vkHandle(); }
		const vzt::Instance*  getInstance() const { return &m_instance; }

		bool update() const;
		bool shouldClose() const { return glfwWindowShouldClose(m_window.get()); }

		static std::vector<const char*> vkExtensions();

	  private:
		vzt::Instance  m_instance;
		SurfaceHandler m_surface;

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
