#pragma once

#include <entt/entt.hpp>

#include "BinaryEngine/Scene/Scene.h"

namespace BinaryEngine {

	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene) : m_Handle(handle), m_Scene(scene) {}
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			if (HasComponent<T>())
			{
				return GetComponent<T>();
			}

			return m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			return m_Scene->m_Registry.get<T>(m_Handle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<T>(m_Handle);
		}

		template<typename T>
		void RemoveComponent()
		{
			if (!HasComponent<T>())
			{
				return;
			}

			m_Scene->m_Registry.remove<T>(m_Handle);
		}

		operator bool() const { return m_Handle != entt::null; }
		operator entt::entity() const { return m_Handle; }

		bool operator==(const Entity& other) const
		{
			return m_Handle == other.m_Handle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

	private:
		entt::entity m_Handle{ entt::null };
		Scene* m_Scene{ nullptr };
	};

}