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

	SurfaceHandler::SurfaceHandler(SurfaceHandler&& other)
	{
		std::swap(m_instance, other.m_instance);
		std::swap(m_surface, other.m_surface);
	}
	SurfaceHandler& SurfaceHandler::operator=(SurfaceHandler&& other)
	{
		std::swap(m_instance, other.m_instance);
		std::swap(m_surface, other.m_surface);

		return *this;
	}

	SurfaceHandler::~SurfaceHandler()
	{
		if (m_surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(m_instance->vkHandle(), m_surface, nullptr);
		}
	}

	Window::Window(const std::string& name, const uint32_t width, const uint32_t height)
	    : m_width(width), m_height(height)
	{
		if (!glfwInit())
			throw std::runtime_error("Failed to load glfw.");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_window = GLFWwindowPtr(glfwCreateWindow(m_width, m_height, name.c_str(), nullptr, nullptr),
		                         [](GLFWwindow* window) { glfwDestroyWindow(window); });

		glfwSetWindowUserPointer(m_window.get(), this);
		glfwSetFramebufferSizeCallback(m_window.get(), [](GLFWwindow* window, int width, int height) {
			Window* windowHandle = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onFramebufferSizeChanged();
		});

		glfwSetCursorPosCallback(m_window.get(), [](GLFWwindow* window, double xPos, double yPos) {
			Window* windowHandle = reinterpret_cast<vzt::Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onMousePosChanged(Vec2{static_cast<float>(xPos), static_cast<float>(yPos)});
		});

		glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int /* scancode */, int action, int mods) {
			Window* windowHandle = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onKeyAction(static_cast<KeyCode>(key), static_cast<KeyAction>(action),
			                          static_cast<KeyModifier>(mods));
		});

		glfwSetMouseButtonCallback(m_window.get(), [](GLFWwindow* window, int key, int action, int mods) {
			Window* windowHandle = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			windowHandle->onMouseButton(static_cast<MouseButton>(key), static_cast<KeyAction>(action),
			                            static_cast<KeyModifier>(mods));
		});

		m_instance = Instance("Vazteran", vkExtensions());

		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(m_instance.vkHandle(), m_window.get(), nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface!");
		m_surface = SurfaceHandler(&m_instance, surface);
	}

	Window::~Window() { glfwTerminate(); }

	void Window::onFramebufferSizeChanged() const { m_onFrameBufferChangedCallback(); }

	void Window::onKeyAction(KeyCode code, KeyAction action, KeyModifier modifiers)
	{
		if (m_onKeyActionCallback)
		{
			m_onKeyActionCallback(code, action, modifiers);
		}
	}

	void Window::onMousePosChanged(const Vec2 pos)
	{
		static bool firstChange = true;
		if (firstChange)
		{
			firstChange    = false;
			m_lastMousePos = pos;
		}

		if (m_onMousePosChangedCallback)
			m_onMousePosChangedCallback({pos.x - m_lastMousePos.x, m_lastMousePos.y - pos.y});

		m_lastMousePos = pos;
	}

	void Window::onMouseButton(MouseButton code, KeyAction action, KeyModifier modifiers)
	{
		if (m_onMouseButtonCallback)
			m_onMouseButtonCallback(code, action, modifiers);
	}

	void Window::setOnFrameBufferChangedCallback(OnFrameBufferChangedCallback callback)
	{
		m_onFrameBufferChangedCallback = std::move(callback);
	}

	void Window::setOnKeyActionCallback(OnKeyActionCallback callback) { m_onKeyActionCallback = std::move(callback); }

	void Window::setOnMousePosChangedCallback(OnMousePosChangedCallback callback)
	{
		m_onMousePosChangedCallback = std::move(callback);
	}

	void Window::setOnMouseButtonCallback(OnMouseButtonCallback callback)
	{
		m_onMouseButtonCallback = std::move(callback);
	}

	Size2D<uint32_t> Window::getFrameBufferSize() const
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

	bool Window::update() const
	{
		glfwPollEvents();

		return shouldClose();
	}

	std::vector<const char*> Window::vkExtensions()
	{
		uint32_t     glfwExtensionCount = 0;
		const char** glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions;
		extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

		return extensions;
	}

} // namespace vzt
