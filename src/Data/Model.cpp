#include "Vazteran/Data/Model.hpp"
#include <iostream>

namespace vzt
{
	Model::Model(const fs::path &modelPath) : m_mesh(modelPath) {}

	Model::~Model() = default;

	vzt::AABB Model::BoundingBox() const
	{
		auto      movedAABB            = m_mesh.BoundingBox();
		glm::mat4 transformationMatrix = ModelMatrix();
		for (auto &position : movedAABB.Vertices())
		{
			position = (transformationMatrix * glm::vec4(position, 1.f));
		}
		movedAABB.Refresh();

		return movedAABB;
	}

	glm::mat4 Model::ModelMatrix() const
	{
		auto translated         = glm::translate(vzt::Mat4(1.0), m_position);
		auto translatedRotateX  = glm::rotate(translated, m_rotation.x, vzt::Vec3(1., 0., 0.));
		auto translatedRotateXY = glm::rotate(translatedRotateX, m_rotation.y, vzt::Vec3(0., 1., 0.));
		return glm::rotate(translatedRotateXY, m_rotation.z, vzt::Vec3(0., 0., 1.));
	}
} // namespace vzt
