#ifndef VAZTERAN_UI_HPP
#define VAZTERAN_UI_HPP

#include <imgui.h>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace vzt {
	class Instance;
	class LogicalDevice;
	class RenderPass;

	class UiRenderer {
	public:	
		UiRenderer(GLFWwindow* window, Instance* instance, LogicalDevice* device, RenderPass* renderPass);
		
		void Update() const;

		~UiRenderer();
	private:
		LogicalDevice* m_device;
		VkDescriptorPool m_descriptorPool;
	};
}

#endif // VAZTERAN_UI_HPP