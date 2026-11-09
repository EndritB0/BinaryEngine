#pragma once

#include <memory>
#include <string>

#include <entt/entt.hpp>

#include "BinaryEngine/Core/Timestep.h"
#include "BinaryEngine/Scene/SceneSpecification.h"

namespace BinaryEngine {

	class AssetManager;
	class Entity;
	class PhysicsBody;
	class PhysicsWorld;
	class Renderer;

	class Scene {
	public:
		Scene();
		explicit Scene(const SceneSpecification& specification);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnFixedUpdate(TimeStep fixedStep);
		void OnUpdate(TimeStep dt, float interpolationAlpha);
		void OnRender(Renderer& renderer, AssetManager& assetManager);

		PhysicsWorld& GetPhysicsWorld();
		const PhysicsWorld& GetPhysicsWorld() const;
		PhysicsBody GetPhysicsBody(Entity entity);

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		friend class Entity;

		std::unique_ptr<PhysicsWorld> m_PhysicsWorld;
		entt::registry m_Registry;
	};

}
