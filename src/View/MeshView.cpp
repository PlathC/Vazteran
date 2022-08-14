#include "Vazteran/View/MeshView.hpp"
#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Core/Type.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Light.hpp"
#include "Vazteran/Data/Mesh.hpp"
#include "Vazteran/Data/Transform.hpp"
#include "Vazteran/Renderer/ShaderLibrary.hpp"
#include "Vazteran/System/Scene.hpp"

namespace vzt
{
	VertexInputDescription TriangleVertexInput::getInputDescription()
	{
		constexpr VkVertexInputBindingDescription bindingDescription{0, sizeof(vzt::TriangleVertexInput),
		                                                             VK_VERTEX_INPUT_RATE_VERTEX};

		auto attributeDescriptions        = std::vector<VkVertexInputAttributeDescription>(3);
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset   = offsetof(vzt::TriangleVertexInput, position);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset   = offsetof(vzt::TriangleVertexInput, textureCoordinates);

		attributeDescriptions[2].binding  = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset   = offsetof(vzt::TriangleVertexInput, normal);

		return vzt::VertexInputDescription{bindingDescription, std::move(attributeDescriptions)};
	}

	struct MeshDeviceData
	{
		Buffer      vertexBuffer;
		Buffer      subMeshesIndexBuffer;
		std::size_t transformIndex;

		struct SubMeshData
		{
			uint32_t minOffset;
			uint32_t maxOffset;
			uint32_t materialDataIndex;
		};

		struct TextureData
		{
			ImageView imageView;
			Texture   texture;
		};
		std::vector<TextureData> textureData;
		std::vector<SubMeshData> subMeshData;
	};

	struct GenericMaterial
	{
		Vec4 diffuse; // diffuse + shininess

		static GenericMaterial fromMaterial(const Material& original)
		{
			return GenericMaterial{glm::vec4(glm::vec3(original.color), original.shininess)};
		}
	};

	struct Transforms
	{
		Mat4 modelViewMatrix;
		Mat4 projectionMatrix;
		Mat4 normalMatrix;
	};

	MeshView::MeshView(uint32_t imageNb, Scene& scene, ShaderLibrary& library)
	    : View(imageNb), m_scene(&scene), m_library(&library)
	{
		m_meshDescriptorLayout = vzt::DescriptorLayout();
		m_meshDescriptorLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
		m_meshDescriptorLayout.addBinding(1, vzt::DescriptorType::UniformBuffer);
		m_meshDescriptorLayout.addBinding(2, vzt::DescriptorType::CombinedSampler);

		createPipeline();
	}

	MeshView::~MeshView() {}

	void MeshView::apply(RenderGraph& graph, AttachmentHandle& position, AttachmentHandle& normal,
	                     AttachmentHandle& albedo, AttachmentHandle& depth)
	{
		auto& geometryBuffer = graph.addPass("G-Buffer", vzt::QueueType::Graphic);
		geometryBuffer.addColorOutput(position, "Position");
		geometryBuffer.addColorOutput(normal, "Normal");
		geometryBuffer.addColorOutput(albedo, "Albedo");
		geometryBuffer.setDepthStencilOutput(depth, "Depth");

		geometryBuffer.setConfigureFunction(
		    [this](const vzt::PipelineContextSettings& settings) { configure(settings); });

		geometryBuffer.setRecordFunction(
		    [this](uint32_t imageId, VkCommandBuffer cmd, const std::vector<VkDescriptorSet>& engineDescriptorSets) {
			    record(imageId, cmd, engineDescriptorSets);
		    });
	}

	void MeshView::refresh() { createPipeline(); }

	void MeshView::configure(const PipelineContextSettings& settings)
	{
		m_scene->forAll<MeshDeviceData>([&](Entity entity) { entity.remove<MeshDeviceData>(); });

		m_device          = settings.device;
		m_transformNumber = 0;
		m_meshDescriptorLayout.configure(settings.device);

		m_descriptorPool = vzt::DescriptorPool(
		    m_device, {vzt::DescriptorType::UniformBuffer, vzt::DescriptorType::CombinedSampler}, 512);

		m_transformOffsetSize = static_cast<uint32_t>(m_device->computeUniformOffsetAlignment<Transforms>());
		m_transformBuffer     = Buffer(m_device, std::vector<uint8_t>(m_maxSupportedMesh * m_transformOffsetSize),
		                               BufferUsage::UniformBuffer, MemoryUsage::PreferDevice, true);

		m_materialInfoOffsetSize = static_cast<uint32_t>(m_device->computeUniformOffsetAlignment<Material>());
		m_materialInfoBuffer     = Buffer(m_device, std::vector<uint8_t>(m_maxSupportedMesh * m_materialInfoOffsetSize),
		                                  BufferUsage::UniformBuffer, MemoryUsage::PreferDevice, true);

		m_scene->forAll<Mesh>([&](Entity entity) { addMesh(*entity.registry(), entity.entity()); });
		m_connections.emplace_back(m_scene->onConstruct<Mesh, &MeshView::addMesh>(*this));
		m_connections.emplace_back(m_scene->onUpdate<Mesh, &MeshView::updateMesh>(*this));

		m_scene->forAll<MainCamera>([&](Entity entity) { updateCamera(*entity.registry(), entity.entity()); });
		m_connections.emplace_back(m_scene->onConstruct<MainCamera, &MeshView::updateCamera>(*this));
		m_connections.emplace_back(m_scene->onUpdate<MainCamera, &MeshView::updateCamera>(*this));

		m_pipeline.configure(settings);
	}

	void MeshView::record(uint32_t imageId, VkCommandBuffer cmd,
	                      const std::vector<VkDescriptorSet>& engineDescriptorSets) const
	{
		m_pipeline.bind(cmd);
		if (!engineDescriptorSets.empty())
			m_pipeline.bind(cmd, engineDescriptorSets);

		m_scene->forAll<Mesh>([&](Entity entity) {
			const auto&  deviceData      = entity.get<MeshDeviceData>();
			VkBuffer     vertexBuffers[] = {deviceData.vertexBuffer.vkHandle()};
			VkDeviceSize offsets[]       = {0};

			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
			const auto& indexBuffer = deviceData.subMeshesIndexBuffer;
			const auto& submeshes   = deviceData.subMeshData;
			for (const auto& subMesh : submeshes)
			{
				vkCmdBindIndexBuffer(cmd, indexBuffer.vkHandle(), subMesh.minOffset * sizeof(uint32_t),
				                     VK_INDEX_TYPE_UINT32);

				m_descriptorPool.bind((subMesh.materialDataIndex) * m_imageNb + imageId, cmd,
				                      VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout());

				vkCmdDrawIndexed(cmd, subMesh.maxOffset - subMesh.minOffset, 1, 0, 0, 0);
			}
		});
	}

	void MeshView::createPipeline()
	{
		vzt::Program triangleProgram{};
		triangleProgram.setShader(m_library->get("./shaders/triangle.vert"));
		triangleProgram.setShader(m_library->get("./shaders/triangle.frag"));
		m_pipeline = {std::move(triangleProgram), m_meshDescriptorLayout,
		              vzt::TriangleVertexInput::getInputDescription()};
	}

	void MeshView::addMesh(entt::registry& registry, entt::entity entity)
	{
		Entity      meshEntity = {registry, entity};
		const Mesh& mesh       = meshEntity.get<Mesh>();

		const auto& vertices = mesh.vertices;
		const auto& normals  = mesh.normals;
		const auto& uvs      = mesh.uvs;

		std::vector<TriangleVertexInput> verticesData;
		verticesData.reserve(vertices.size());
		for (std::size_t i = 0; i < vertices.size(); i++)
			verticesData.emplace_back(TriangleVertexInput{vertices[i], uvs[i], normals[i]});

		MeshDeviceData& meshData = meshEntity.emplace<MeshDeviceData>();

		meshData.vertexBuffer   = Buffer(m_device, Span<TriangleVertexInput>(verticesData), BufferUsage::VertexBuffer);
		meshData.transformIndex = m_transformNumber;
		auto subMeshRawIndices  = mesh.getVertexIndices();
		auto subMeshMaterialIndices = mesh.getMaterialIndices();

		meshData.subMeshData.reserve(subMeshRawIndices.size());
		std::vector<uint32_t> submeshIndices;
		submeshIndices.reserve(subMeshRawIndices.size());
		for (std::size_t i = 0; i < subMeshRawIndices.size(); i++)
		{
			MeshDeviceData::SubMeshData data;
			data.minOffset         = static_cast<uint32_t>(submeshIndices.size());
			data.maxOffset         = static_cast<uint32_t>(submeshIndices.size() + subMeshRawIndices[i].size());
			data.materialDataIndex = m_materialNb + subMeshMaterialIndices[i];
			meshData.subMeshData.emplace_back(std::move(data));

			submeshIndices.insert(submeshIndices.end(), subMeshRawIndices[i].begin(), subMeshRawIndices[i].end());

			meshData.subMeshesIndexBuffer = Buffer(m_device, Span<uint32_t>(submeshIndices), BufferUsage::IndexBuffer);

			IndexedUniform<BufferSpan> bufferDescriptors;
			const std::size_t          transformFrom = m_transformNumber * m_transformOffsetSize;
			bufferDescriptors[0] = m_transformBuffer.get(transformFrom, transformFrom + sizeof(Transforms));

			const auto& materials = mesh.materials;
			if (!materials.empty())
				meshData.textureData.resize(meshData.textureData.size() + materials.size());

			for (const auto& material : materials)
			{
				const std::size_t materialFrom = m_materialNb * m_materialInfoOffsetSize;
				bufferDescriptors[1] = m_materialInfoBuffer.get(materialFrom, materialFrom + sizeof(GenericMaterial));

				const auto genericMaterial = GenericMaterial::fromMaterial(material);
				m_materialInfoBuffer.update(genericMaterial, materialFrom);

				IndexedUniform<vzt::Texture*> texturesDescriptors;

				auto& textureData = meshData.textureData.back();

				textureData.imageView  = {m_device, material.texture, Format::R8G8B8A8SRGB};
				textureData.texture    = {m_device, &meshData.textureData.back().imageView};
				texturesDescriptors[2] = &textureData.texture;

				m_descriptorPool.allocate(m_imageNb, m_meshDescriptorLayout);
				for (std::size_t i = 0; i < m_imageNb; i++)
					m_descriptorPool.update(m_materialNb * m_imageNb + i, bufferDescriptors, texturesDescriptors);

				m_materialNb++;
			}
		}

		m_transformNumber++;
	}

	void MeshView::updateMesh(entt::registry& registry, entt::entity entity)
	{
		const Entity meshEntity = {registry, entity};
		const Mesh&  mesh       = meshEntity.get<Mesh>();
		const auto&  deviceData = meshEntity.get<MeshDeviceData>();
		for (std::size_t i = 0; i < deviceData.subMeshData.size(); i++)
		{
			const MeshDeviceData::SubMeshData& subMeshData = deviceData.subMeshData[i];
			const auto genericMaterial = GenericMaterial::fromMaterial(mesh.materials[mesh.subMeshes[i].materialIndex]);
			for (std::size_t j = 0; j < m_imageNb; j++)
			{
				m_materialInfoBuffer.update(genericMaterial, subMeshData.materialDataIndex * m_materialInfoOffsetSize);
			}
		}
	}

	void MeshView::updateCamera(entt::registry& registry, entt::entity entity)
	{
		const Entity& cameraEntity = {registry, entity};

		const auto& [setup, view]   = cameraEntity.get<Camera, Transform>();
		const Mat4 viewMatrix       = setup.getViewMatrix(view);
		Mat4       projectionMatrix = setup.getProjectionMatrix();
		projectionMatrix[1][1] *= -1;

		Transforms transforms;
		transforms.projectionMatrix = projectionMatrix;

		std::size_t currentMaterialIndex = 0;
		m_scene->forAll<Mesh>([&](Entity entity) {
			const auto&      deviceData = entity.get<MeshDeviceData>();
			const Transform* transform  = entity.try_get<Transform>();

			const Mat4 modelMatrix     = transform ? transform->get() : Mat4(1.f);
			const Mat4 modelViewMatrix = viewMatrix * modelMatrix;

			transforms.modelViewMatrix = modelViewMatrix;
			transforms.normalMatrix    = glm::transpose(glm::inverse(modelViewMatrix));
			m_transformBuffer.update(transforms, deviceData.transformIndex * m_transformOffsetSize);
		});
	}
} // namespace vzt
