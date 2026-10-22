#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string_view>
#include <vector>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

namespace Sandbox {

	class StressDemoState : public BinaryEngine::State {
	public:
		StressDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~StressDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		enum class BatchingMode {
			Sorted,
			Interleaved
		};

		struct StressSprite {
			BinaryEngine::Transform SpriteTransform{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } };
			BinaryEngine::Vector2f Velocity{ 0.0f, 0.0f };
			float SpinDegreesPerSecond{ 0.0f };
			std::size_t TextureIndex{ 0 };
		};

		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		void CreateColorTextures();
		void ChangeSpriteCount(std::int64_t change);
		void SpawnSprite(std::size_t spriteIndex);
		float GetSortZ(std::size_t textureIndex) const;
		std::string_view GetBatchingModeName() const;
		void ApplyBatchingMode();
		void MoveSprites(float seconds);
		void UpdateHud();
		void RecordFrame(float seconds);
		void FinishMeasurement();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;

		BinaryEngine::AssetHandle m_CharacterTextureHandle;
		std::array<BinaryEngine::Texture2D, 3> m_ColorTextures;

		std::vector<StressSprite> m_Sprites;
		std::mt19937 m_RandomEngine;
		BatchingMode m_BatchingMode{ BatchingMode::Sorted };
		bool m_MotionEnabled{ false };

		std::uint32_t m_WarmUpFramesLeft{ 0 };
		std::uint32_t m_MeasuredFrames{ 0 };
		float m_MeasuredSeconds{ 0.0f };
		std::uint32_t m_LastQuadCount{ 0 };
		std::uint32_t m_LastDrawCalls{ 0 };
	};

}
