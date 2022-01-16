#ifndef VAZTERAN_MODEL_HPP
#define VAZTERAN_MODEL_HPP

#include <filesystem>
namespace fs = std::filesystem;

#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Data/Mesh.hpp"

namespace vzt
{
	class Model
	{
	  public:
		Model(const fs::path &modelPath);
		~Model();

		vzt::AABB        BoundingBox() const;
		vzt::Mesh       &Mesh() { return m_mesh; };
		const vzt::Mesh &CMesh() const { return m_mesh; };

		glm::mat4  ModelMatrix() const;
		glm::vec3 &Rotation() { return m_rotation; }
		glm::vec3  CRotation() const { return m_rotation; }
		glm::vec3 &Position() { return m_position; }
		glm::vec3  CPosition() const { return m_position; }

	  private:
		vzt::Mesh m_mesh;

		vzt::Vec3 m_position = {0.f, 0.f, 0.f};

		// TODO: Quaternion
		vzt::Vec3 m_rotation = {0.f, 0.f, 0.f};
	};
} // namespace vzt

#endif // VAZTERAN_MODEL_HPP
