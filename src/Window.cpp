#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	SurfaceHandler::SurfaceHandler(vzt::Instance* instance, VkSurfaceKHR surface)
	    : m_instance(instance), m_surface(surface)
	{
	}

	SurfaceHandler::~SurfaceHandler() { vkDestroySurfaceKHR(m_instance->vkHandle(), m_surface, nullptr); }

	Window::Window(std::string_view name, const uint32_t width, const uint32_t height,
	               vzt::OnFrameBufferChangedCallback onFrameBufferChangedCallback)
	    : m_width(width), m_height(height), m_onFrameBufferChangedCallback(std::move(onFrameBufferChangedCallback))
	{
		if (!glfwInit())
			throw std::runtime_error("Failed to load glfw.");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_window = GLFWwindowPtr(glfwCreateWindow(m_width, m_height, std::string(name).c_str(), nullptr, nullptr),
		                         [](GLFWwindow* window) { glfwDestroyWindow(window); });

		glfwSetWindowUserPointer(m_window.get(), this);
		glfwSetFramebufferSizeCallback(m_window.get(), [](GLFWwindow* window, int width, int height) {
			vzt::Window* windowHandle = reinterpret_cast<vzt::Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onFramebufferSizeChanged();
		});

		glfwSetCursorPosCallback(m_window.get(), [](GLFWwindow* window, double xPos, double yPos) {
			vzt::Window* windowHandle = reinterpret_cast<vzt::Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onMousePosChanged(vzt::Dvec2{xPos, yPos});
		});

		glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int /* scancode */, int action, int mods) {
			vzt::Window* windowHandle = reinterpret_cast<vzt::Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onKeyAction(static_cast<vzt::KeyCode>(key), static_cast<vzt::KeyAction>(action),
			                          static_cast<vzt::KeyModifier>(mods));
		});

		glfwSetMouseButtonCallback(m_window.get(), [](GLFWwindow* window, int key, int action, int mods) {
			vzt::Window* windowHandle = reinterpret_cast<vzt::Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onMouseButton(static_cast<vzt::MouseButton>(key), static_cast<vzt::KeyAction>(action),
			                            static_cast<vzt::KeyModifier>(mods));
		});
	}

	Window::~Window() = default;

	void Window::onFramebufferSizeChanged() const { m_onFrameBufferChangedCallback(); }

	void Window::onKeyAction(vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers)
	{
		if (m_onKeyActionCallback)
		{
			m_onKeyActionCallback(code, action, modifiers);
		}
	}

	void Window::onMousePosChanged(const vzt::Dvec2 pos)
	{
		static bool firstChange = true;
		if (firstChange)
		{
			firstChange    = false;
			m_lastMousePos = pos;
		}

		if (m_onMousePosChangedCallback)
		{
			m_onMousePosChangedCallback(vzt::Dvec2{pos.x - m_lastMousePos.x, m_lastMousePos.y - pos.y});
		}

		m_lastMousePos = pos;
	}

	void Window::onMouseButton(vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers)
	{
		if (m_onMouseButtonCallback)
		{
			m_onMouseButtonCallback(code, action, modifiers);
		}
	}

	void Window::setOnKeyActionCallback(OnKeyActionCallback callback) { m_onKeyActionCallback = callback; }

	void Window::setOnMousePosChangedCallback(OnMousePosChangedCallback callback)
	{
		m_onMousePosChangedCallback = callback;
	}

	void Window::setOnMouseButtonCallback(OnMouseButtonCallback callback) { m_onMouseButtonCallback = callback; }

	vzt::Size2D<uint32_t> Window::getFrameBufferSize() const
	{
		int frameBufferWidth = 0, frameBufferHeight = 0;
		glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
		while (frameBufferWidth == 0 || frameBufferHeight == 0)
		{
			glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
			glfwWaitEvents();
		}
		return {static_cast<uint32_t>(frameBufferWidth), static_cast<uint32_t>(frameBufferHeight)};
	}

	VkSurfaceKHR Window::getSurface(vzt::Instance* instance)
	{
		if (!m_surface)
		{
			VkSurfaceKHR surface;
			if (glfwCreateWindowSurface(instance->vkHandle(), m_window.get(), nullptr, &surface) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create window surface!");
			}
			m_surface = std::make_unique<SurfaceHandler>(instance, surface);
		}

		return m_surface->vkHandle();
	}

	bool Window::update()
	{
		glfwPollEvents();

		return shouldClose();
	}

	std::vector<const char*> Window::vkExtensions() const
	{
		uint32_t     glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions;
		extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

		return extensions;
	}

} // namespace vzt
