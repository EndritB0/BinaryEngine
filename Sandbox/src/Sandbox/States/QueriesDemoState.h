#pragma once

#include <string_view>
#include <vector>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Physics/PhysicsTypes.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

#include "Sandbox/PhysicsDemoCommon.h"

namespace Sandbox {

	class QueriesDemoState : public BinaryEngine::State {
	public:
		QueriesDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~QueriesDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnFixedUpdate(BinaryEngine::TimeStep fixedStep) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		bool OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event);
		void CreateLevel();
		void CreateBoxBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, float rotationDegrees, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider);
		void CreateBall(BinaryEngine::Vector2f position);
		BinaryEngine::Entity CreateCapsuleBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, const BinaryEngine::RigidBodySpecification& body);
		void SpawnCapsule(BinaryEngine::Vector2f position);
		void RemoveSpawnedBodies();
		void DrawCapsuleBodies(BinaryEngine::Renderer& renderer);
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;

		std::vector<BinaryEngine::Entity> m_SpawnedEntities;
		float m_LastSpawnedMass{ 0.0f };
		bool m_ShowOutlines{ false };
	};

}
