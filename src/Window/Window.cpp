#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Window/Window.hpp"

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
			windowHandle->processFrameBufferResize(Uvec2{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
		});

		glfwSetCursorPosCallback(m_window.get(), [](GLFWwindow* window, double xPos, double yPos) {
			Window* windowHandle = reinterpret_cast<vzt::Window*>(glfwGetWindowUserPointer(window));
			windowHandle->processCursorInput(Vec2{static_cast<float>(xPos), static_cast<float>(yPos)});
		});

		glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int /* scancode */, int action, int mods) {
			Window* windowHandle = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			windowHandle->processKeyInput(static_cast<KeyCode>(key), static_cast<KeyAction>(action),
			                              static_cast<KeyModifier>(mods));
		});

		glfwSetMouseButtonCallback(m_window.get(), [](GLFWwindow* window, int key, int action, int mods) {
			Window* windowHandle = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			windowHandle->processMouseButtonInput(static_cast<MouseButton>(key), static_cast<KeyAction>(action),
			                                      static_cast<KeyModifier>(mods));
		});

		m_instance = Instance("Vazteran", vkExtensions());

		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(m_instance.vkHandle(), m_window.get(), nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface!");
		m_surface = SurfaceHandler(&m_instance, surface);
	}

	Window::~Window() { glfwTerminate(); }

	Uvec2 Window::getFrameBufferSize() const
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

	void Window::processFrameBufferResize(Uvec2 size)
	{
		m_dispatcher.enqueue(FrameBufferResize{size});
		m_triggerUserInput = true;
	}

	void Window::processCursorInput(Vec2 pos)
	{
		m_inputs.updateMousePosition(pos);
		m_triggerUserInput = true;
	}

	void Window::processKeyInput(KeyCode key, KeyAction action, KeyModifier mods)
	{
		if (m_triggerUserInput)
			m_inputs.modifiers = KeyModifier::None;

		m_inputs.keys.set(key, action);
		if (mods == KeyModifier::None)
			m_inputs.modifiers = KeyModifier::None;
		else
			m_inputs.modifiers = m_inputs.modifiers | mods;
		m_triggerUserInput = true;
	}

	void Window::processMouseButtonInput(MouseButton key, KeyAction action, KeyModifier mods)
	{
		if (m_triggerUserInput)
			m_inputs.modifiers = KeyModifier::None;

		m_inputs.mouseButtons.set(key, action);

		if (mods == KeyModifier::None)
			m_inputs.modifiers = KeyModifier::None;
		else
			m_inputs.modifiers = m_inputs.modifiers | mods;
		m_triggerUserInput = true;
	}

	bool Window::update()
	{
		m_inputs.reset();
		glfwPollEvents();

		const bool isOver = shouldClose();
		if (!isOver)
		{
			if (m_triggerUserInput)
				m_dispatcher.enqueue(m_inputs);
			m_dispatcher.update();
			if (!m_triggerUserInput)
				m_inputs.modifiers = KeyModifier::None;
		}

		return isOver;
	}

	std::vector<const char*> Window::vkExtensions()
	{
		uint32_t     glfwExtensionCount = 0;
		const char** glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions;
		extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

		return extensions;
	}

	bool Window::shouldClose() const { return glfwWindowShouldClose(m_window.get()); }
} // namespace vzt
