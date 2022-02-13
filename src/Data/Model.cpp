#include "Vazteran/Data/Model.hpp"
#include <iostream>

namespace vzt
{
	Model::Model(const fs::path& modelPath, vzt::ModelUpdateCallback callback)
	    : m_mesh(modelPath), m_updateCallback(std::move(callback))
	{
	}

	Model::~Model() = default;

	vzt::AABB Model::boundingBox() const
	{
		auto      movedAABB            = m_mesh.boundingBox();
		glm::mat4 transformationMatrix = getModelMatrix();
		for (auto& position : movedAABB.getVertices())
		{
			position = (transformationMatrix * glm::vec4(position, 1.f));
		}
		movedAABB.refresh();

		return movedAABB;
	}

	glm::mat4 Model::getModelMatrix() const
	{
		auto translated         = glm::translate(vzt::Mat4(1.0), m_position);
		auto translatedRotateX  = glm::rotate(translated, m_rotation.x, vzt::Vec3(1., 0., 0.));
		auto translatedRotateXY = glm::rotate(translatedRotateX, m_rotation.y, vzt::Vec3(0., 1., 0.));
		return glm::rotate(translatedRotateXY, m_rotation.z, vzt::Vec3(0., 0., 1.));
	}

	void Model::update()
	{
		if (m_updateCallback)
		{
			m_updateCallback(this);
		}
	}
} // namespace vzt
