#include "Vazteran/System/Scene.hpp"

namespace vzt
{
	template <class... ComponentTypes>
	void Scene::forAll(ForAllFunction forAllFunction)
	{
		auto entities = m_registry.view<ComponentTypes...>();
		for (const auto entity : entities)
			forAllFunction({m_registry, entity});
	}
	template <class... ComponentTypes>
	void Scene::forAll(ForAllIndexedFunction forallIndexedFunction)
	{
		auto entities = m_registry.view<ComponentTypes...>();

		std::size_t i = 0;
		for (const auto entity : entities)
			forallIndexedFunction(i++, entity);
	}
} // namespace vzt