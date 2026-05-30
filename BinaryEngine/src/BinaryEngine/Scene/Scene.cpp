#include "pch.h"
#include "BinaryEngine/Scene/Scene.h"

#include "BinaryEngine/Scene/Components.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	BinaryEngine::Scene::Scene()
	{
		CORE_INFO("[Scene] Scene Initialised");
	}

	Scene::~Scene()
	{
		CORE_INFO("[Scene] Scene Shutdown");
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		entity.AddComponent<TransformComponent>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate([[maybe_unused]] TimeStep dt)
	{

	}

	void Scene::OnRender()
	{

	}

}