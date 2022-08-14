#ifndef VAZTERAN_MESHVIEW_HPP
#define VAZTERAN_MESHVIEW_HPP

#include <vector>

#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Core/Event.hpp"
#include "Vazteran/System/System.hpp"
#include "Vazteran/View/View.hpp"

namespace vzt
{
	class Device;
	class Scene;
	class ShaderLibrary;

	struct TriangleVertexInput
	{
		Vec3 position;
		Vec2 textureCoordinates;
		Vec3 normal;

		static VertexInputDescription getInputDescription();
	};

	class MeshView : public View
	{
	  public:
		MeshView(uint32_t imageNb, Scene& scene, ShaderLibrary& library);

		MeshView(const MeshView&)            = delete;
		MeshView& operator=(const MeshView&) = delete;

		MeshView(MeshView&& other) noexcept            = default;
		MeshView& operator=(MeshView&& other) noexcept = default;

		~MeshView();

		void apply(RenderGraph& graph, AttachmentHandle& position, AttachmentHandle& normal, AttachmentHandle& albedo,
		           AttachmentHandle& depth);
		void refresh() override;
		void configure(const PipelineContextSettings& settings) override;
		void record(uint32_t imageId, VkCommandBuffer cmd,
		            const std::vector<VkDescriptorSet>& engineDescriptorSets) const override;

	  private:
		void createPipeline();
		void addMesh(entt::registry& registry, entt::entity entity);
		void updateMesh(entt::registry& registry, entt::entity entity);
		void updateCamera(entt::registry& registry, entt::entity entity);

		const Device* m_device;

		DescriptorPool   m_descriptorPool;
		DescriptorLayout m_meshDescriptorLayout;

		const std::size_t m_maxSupportedMesh = 128;

		Buffer   m_materialInfoBuffer;
		uint32_t m_materialNb             = 0;
		uint32_t m_materialInfoOffsetSize = 0;

		Buffer   m_transformBuffer;
		uint32_t m_transformNumber     = 0;
		uint32_t m_transformOffsetSize = 0;

		Scene*                  m_scene;
		std::vector<Connection> m_connections;

		ShaderLibrary*  m_library;
		GraphicPipeline m_pipeline;
	};

} // namespace vzt

#endif // VAZTERAN_MESHVIEW_HPP
