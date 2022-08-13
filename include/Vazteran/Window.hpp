#ifndef VAZTERAN_WINDOW_HPP
#define VAZTERAN_WINDOW_HPP

#include <functional>
#include <memory>

#define GLFW_INCLUDE_VULKAN

#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Core/Event.hpp"
#include "Vazteran/Math/Math.hpp"
#include "Vazteran/Ui/Inputs.hpp"

namespace vzt
{
	class Instance;
	struct SurfaceHandler
	{
	  public:
		SurfaceHandler() = default;
		SurfaceHandler(Instance* instance, VkSurfaceKHR surface);

		SurfaceHandler(const SurfaceHandler&)            = delete;
		SurfaceHandler& operator=(const SurfaceHandler&) = delete;
		SurfaceHandler(SurfaceHandler&&);
		SurfaceHandler& operator=(SurfaceHandler&&);

		VkSurfaceKHR vkHandle() const { return m_surface; }
		~SurfaceHandler();

	  private:
		Instance*    m_instance = nullptr;
		VkSurfaceKHR m_surface  = VK_NULL_HANDLE;
	};

	class Window : public Provider
	{
	  public:
		struct FrameBufferResize
		{
			Uvec2 size;
		};

		Window(const std::string& name, uint32_t width, uint32_t height);
		~Window();

		GLFWwindow*     getWindowHandle() const { return m_window.get(); }
		Uvec2           getFrameBufferSize() const;
		VkSurfaceKHR    getSurface() const { return m_surface.vkHandle(); }
		const Instance* getInstance() const { return &m_instance; }

		bool update();

		// Should not be call by user's code
		void processFrameBufferResize(Uvec2 size);
		void processCursorInput(Vec2 pos);
		void processKeyInput(KeyCode key, KeyAction action, KeyModifier mods);
		void processMouseButtonInput(MouseButton key, KeyAction action, KeyModifier mods);

	  private:
		static std::vector<const char*> vkExtensions();

		bool shouldClose() const;

		Instance       m_instance;
		SurfaceHandler m_surface;

		uint32_t m_width;
		uint32_t m_height;

		using GLFWwindowPtr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;
		GLFWwindowPtr m_window;

		bool   m_triggerUserInput = false;
		Inputs m_inputs{};
	};
} // namespace vzt

#endif // VAZTERAN_WINDOW_HPP
