#pragma once

#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Physics/PhysicsWorldSpecification.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/Scene/SceneSpecification.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

namespace Sandbox {

	class PhysicsDemoState : public BinaryEngine::State {
	public:
		PhysicsDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~PhysicsDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnFixedUpdate(BinaryEngine::TimeStep fixedStep) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene{ BinaryEngine::SceneSpecification{ .Physics{ BinaryEngine::PhysicsWorldSpecification{} } } };
		BinaryEngine::Entity m_HudEntity;
	};

}
