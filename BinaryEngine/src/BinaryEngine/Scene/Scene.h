#pragma once

#include <entt/entt.hpp>

#include "BinaryEngine/Core/Timestep.h"

namespace BinaryEngine {

	class AssetManager;
	class Entity;
	class Renderer;

	class Scene {
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdate(TimeStep dt);
		void OnRender(Renderer& renderer, AssetManager& assetManager);

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		friend class Entity;

		entt::registry m_Registry;
	};

}