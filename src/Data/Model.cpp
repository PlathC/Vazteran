#include "Vazteran/Data/Model.hpp"
#include <iostream>

namespace vzt
{
	Model::Model(const fs::path &modelPath) : m_mesh(modelPath)
	{
	}

	vzt::AABB Model::BoundingBox() const
	{
		// https://stackoverflow.com/a/58630206
		auto movedAABB = m_mesh.BoundingBox();

		glm::mat4 translated = glm::translate(glm::mat4(1.f), m_position);
		glm::mat4 translatedRotateX = glm::rotate(translated, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 translatedRotateXY = glm::rotate(translatedRotateX, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 transformationMatrix = glm::rotate(translatedRotateXY, m_rotation.z, glm::vec3(0., 0., 1.));

		for (auto &position : movedAABB.Vertices())
		{
			position = (transformationMatrix * glm::vec4(position, 1.f));
		}
		movedAABB.Refresh();

		return movedAABB;
	}

	glm::mat4 Model::ModelMatrix() const
	{
		auto translated = glm::translate(glm::mat4(1.0), m_position);
		auto translatedRotateX = glm::rotate(translated, m_rotation.x, glm::vec3(1., 0., 0.));
		auto translatedRotateXY = glm::rotate(translatedRotateX, m_rotation.y, glm::vec3(0., 1., 0.));
		return glm::rotate(translatedRotateXY, m_rotation.z, glm::vec3(0., 0., 1.));
	}
} // namespace vzt
