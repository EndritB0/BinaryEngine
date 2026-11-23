#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Physics/PhysicsQueries.h>
#include <BinaryEngine/Physics/PhysicsTypes.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

#include "Sandbox/PhysicsDemoCommon.h"

namespace Sandbox {

	enum class QueryMode {
		Ray,
		RayAll,
		ShapeCast,
		Overlap
	};

	enum class QueryShapeChoice {
		Circle,
		Box,
		Capsule
	};

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
		BinaryEngine::Vector2f GetCursorWorldPosition() const;
		BinaryEngine::QueryFilter MakeQueryFilter() const;
		BinaryEngine::QueryShape MakeQueryShape(BinaryEngine::Vector2f center) const;
		void CreateLevel();
		void CreateBoxBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, float rotationDegrees, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider);
		void CreateBall(BinaryEngine::Vector2f position);
		BinaryEngine::Entity CreateCapsuleBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, const BinaryEngine::RigidBodySpecification& body);
		void SpawnCapsule(BinaryEngine::Vector2f position);
		void RemoveSpawnedBodies();
		void PickBody(BinaryEngine::Vector2f position);
		std::string RunQuery(BinaryEngine::Renderer& renderer);
		std::string RunRayQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor);
		std::string RunRayAllQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor);
		std::string RunShapeCastQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor);
		std::string RunOverlapQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor);
		void DrawCapsuleBodies(BinaryEngine::Renderer& renderer);
		void DrawHighlight(BinaryEngine::Renderer& renderer, BinaryEngine::Entity entity);
		void UpdateHud(std::string_view queryResult);

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;
		BinaryEngine::AssetHandle m_FontHandle;

		std::vector<BinaryEngine::Entity> m_SpawnedEntities;
		std::vector<BinaryEngine::CastHit> m_RayHits;
		std::vector<BinaryEngine::OverlapHit> m_OverlapHits;
		BinaryEngine::Entity m_PickedEntity;
		BinaryEngine::Vector2f m_EyePosition{ 0.0f, 0.0f };
		QueryMode m_Mode{ QueryMode::Ray };
		QueryShapeChoice m_ShapeChoice{ QueryShapeChoice::Circle };
		float m_ShapeRotationDegrees{ 0.0f };
		std::uint64_t m_RayCastCount{ 0 };
		std::uint64_t m_RayAllCastCount{ 0 };
		std::uint64_t m_ShapeCastCount{ 0 };
		std::uint64_t m_OverlapCount{ 0 };
		std::uint64_t m_PointPickCount{ 0 };
		bool m_IncludeTriggers{ false };
		bool m_ShowOutlines{ false };
	};

}
