#include "pch.h"
#include "Sandbox/States/PhysicsDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Physics/PhysicsWorld.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x1A, 0x22, 0x30 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };

	}

	PhysicsDemoState::PhysicsDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[PhysicsDemoState] Created");
	}

	PhysicsDemoState::~PhysicsDemoState()
	{
		APP_TRACE("[PhysicsDemoState] Destroyed");
	}

	void PhysicsDemoState::OnAttach()
	{
		APP_INFO("[PhysicsDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);

		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer) };
		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::RenderSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("PhysicsHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, s_HudZ };
	}

	void PhysicsDemoState::OnDetach()
	{
		APP_INFO("[PhysicsDemoState] Detached");
	}

	void PhysicsDemoState::OnEvent([[maybe_unused]] BinaryEngine::Event& event)
	{}

	void PhysicsDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		m_Scene.OnFixedUpdate(fixedStep);
	}

	void PhysicsDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		m_Scene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
		UpdateHud();
	}

	void PhysicsDemoState::OnRender()
	{
		m_Context.renderer.BeginScene(m_Camera);
		m_Scene.OnRender(m_Context.renderer, m_Context.assetManager);
		m_Context.renderer.EndScene();
	}

	void PhysicsDemoState::UpdateHud()
	{
		const BinaryEngine::PhysicsWorld& world{ m_Scene.GetPhysicsWorld() };
		const BinaryEngine::Vector2f gravity{ world.GetGravity() };
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::format("World {}   Steps {}   Gravity ({:.2f}, {:.2f})   Bodies {}", world.IsValid() ? "active" : "inactive", world.GetStepCount(), gravity.x, gravity.y, world.GetBodyCount());
	}

}
