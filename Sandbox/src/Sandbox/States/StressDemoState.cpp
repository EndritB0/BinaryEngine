#include "pch.h"
#include "Sandbox/States/StressDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/TextureRegion.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_CharacterTexturePath{ "./resources/character/character_idle.png" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x20, 0x20, 0x20 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr std::size_t s_InitialSpriteCount{ 20000 };
		constexpr std::int64_t s_SmallCountStep{ 10000 };
		constexpr std::int64_t s_LargeCountStep{ 100000 };
		constexpr std::int64_t s_MaxSpriteCount{ 900000 };
		constexpr std::uint32_t s_WarmUpFrames{ 10 };
		constexpr std::size_t s_TextureCount{ 4 };
		constexpr int s_ColorTextureSize{ 8 };
		constexpr std::size_t s_ColorTexturePixelCount{ static_cast<std::size_t>(s_ColorTextureSize * s_ColorTextureSize) };
		constexpr std::array<BinaryEngine::Color, 3> s_ColorTextureColors{
			BinaryEngine::Color{ 0xDC, 0x3C, 0x3C },
			BinaryEngine::Color{ 0x3C, 0xC8, 0x5A },
			BinaryEngine::Color{ 0xE6, 0xC8, 0x3C },
		};
		constexpr float s_MaxSpeed{ 40.0f };
		constexpr float s_MaxSpinDegreesPerSecond{ 90.0f };
		constexpr float s_MinScale{ 0.75f };
		constexpr float s_MaxScale{ 1.5f };
		const BinaryEngine::TextureRegion s_CharacterRegion{ { 0, 0 }, { 16, 16 } };
	}

	StressDemoState::StressDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[StressDemoState] Created");
	}

	StressDemoState::~StressDemoState()
	{
		APP_TRACE("[StressDemoState] Destroyed");
	}

	void StressDemoState::OnAttach()
	{
		APP_INFO("[StressDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);

		m_CharacterTextureHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>(s_CharacterTexturePath, m_Context.renderer);
		CreateColorTextures();

		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer) };
		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::TextSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("StressHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, 10.0f };

		ChangeSpriteCount(static_cast<std::int64_t>(s_InitialSpriteCount));
	}

	void StressDemoState::OnDetach()
	{
		FinishMeasurement();
		APP_INFO("[StressDemoState] Detached");
	}

	void StressDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void StressDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		const float seconds{ dt.GetSeconds() };
		RecordFrame(seconds);

		if (m_MotionEnabled)
		{
			MoveSprites(seconds);
		}

		UpdateHud();
		m_Scene.OnUpdate(dt);
	}

	void StressDemoState::OnRender()
	{
		BinaryEngine::Renderer& renderer{ m_Context.renderer };
		const std::shared_ptr<BinaryEngine::Texture2D> characterTexture{ m_Context.assetManager.GetAsset<BinaryEngine::Texture2D>(m_CharacterTextureHandle) };

		renderer.BeginScene(m_Camera);

		for (const StressSprite& sprite : m_Sprites)
		{
			if (sprite.TextureIndex == 0)
			{
				if (characterTexture)
				{
					renderer.DrawSprite(*characterTexture, sprite.SpriteTransform, s_CharacterRegion);
				}
			}
			else
			{
				renderer.DrawSprite(m_ColorTextures[sprite.TextureIndex - 1], sprite.SpriteTransform);
			}
		}

		m_Scene.OnRender(renderer, m_Context.assetManager);
		renderer.EndScene();

		const BinaryEngine::Renderer::Statistics& statistics{ renderer.GetStatistics() };
		m_LastQuadCount = statistics.quadCount;
		m_LastDrawCalls = statistics.drawCalls;
	}

	bool StressDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::UpArrow:
			{
				ChangeSpriteCount(s_SmallCountStep);
				return true;
			}

			case BinaryEngine::Key::DownArrow:
			{
				ChangeSpriteCount(-s_SmallCountStep);
				return true;
			}

			case BinaryEngine::Key::RightArrow:
			{
				ChangeSpriteCount(s_LargeCountStep);
				return true;
			}

			case BinaryEngine::Key::LeftArrow:
			{
				ChangeSpriteCount(-s_LargeCountStep);
				return true;
			}

			case BinaryEngine::Key::B:
			{
				if (!event.IsRepeat())
				{
					FinishMeasurement();
					m_BatchingMode = m_BatchingMode == BatchingMode::Sorted ? BatchingMode::Interleaved : BatchingMode::Sorted;
					ApplyBatchingMode();
				}
				return true;
			}

			case BinaryEngine::Key::Space:
			{
				if (!event.IsRepeat())
				{
					FinishMeasurement();
					m_MotionEnabled = !m_MotionEnabled;
				}
				return true;
			}

			default: return false;
		}
	}

	void StressDemoState::CreateColorTextures()
	{
		for (std::size_t textureIndex{ 0 }; textureIndex < m_ColorTextures.size(); textureIndex++)
		{
			const BinaryEngine::Color color{ s_ColorTextureColors[textureIndex] };

			std::array<std::uint8_t, s_ColorTexturePixelCount * 4> pixels{};
			for (std::size_t pixelIndex{ 0 }; pixelIndex < s_ColorTexturePixelCount; pixelIndex++)
			{
				pixels[pixelIndex * 4 + 0] = color.red;
				pixels[pixelIndex * 4 + 1] = color.green;
				pixels[pixelIndex * 4 + 2] = color.blue;
				pixels[pixelIndex * 4 + 3] = color.alpha;
			}

			if (!m_ColorTextures[textureIndex].Create(m_Context.renderer, pixels.data(), { s_ColorTextureSize, s_ColorTextureSize }, "StressColorTexture"))
			{
				APP_ERROR("[StressDemoState] Couldn't create colour texture {}", textureIndex);
			}
		}
	}

	void StressDemoState::ChangeSpriteCount(std::int64_t change)
	{
		const std::int64_t currentCount{ static_cast<std::int64_t>(m_Sprites.size()) };
		const std::size_t requestedCount{ static_cast<std::size_t>(std::clamp(currentCount + change, std::int64_t{ 0 }, s_MaxSpriteCount)) };
		if (requestedCount == m_Sprites.size())
		{
			return;
		}

		FinishMeasurement();

		if (requestedCount < m_Sprites.size())
		{
			m_Sprites.resize(requestedCount);
			return;
		}

		m_Sprites.reserve(requestedCount);
		for (std::size_t spriteIndex{ m_Sprites.size() }; spriteIndex < requestedCount; spriteIndex++)
		{
			SpawnSprite(spriteIndex);
		}
	}

	void StressDemoState::SpawnSprite(std::size_t spriteIndex)
	{
		const BinaryEngine::Vector2f halfView{ m_Camera.GetWorldViewSize() * 0.5f };
		const BinaryEngine::Vector3f cameraPosition{ m_Camera.GetPosition() };

		std::uniform_real_distribution<float> horizontal{ cameraPosition.x - halfView.x, cameraPosition.x + halfView.x };
		std::uniform_real_distribution<float> vertical{ cameraPosition.y - halfView.y, cameraPosition.y + halfView.y };
		std::uniform_real_distribution<float> speed{ -s_MaxSpeed, s_MaxSpeed };
		std::uniform_real_distribution<float> spin{ -s_MaxSpinDegreesPerSecond, s_MaxSpinDegreesPerSecond };
		std::uniform_real_distribution<float> rotation{ 0.0f, 360.0f };
		std::uniform_real_distribution<float> scale{ s_MinScale, s_MaxScale };

		StressSprite& sprite{ m_Sprites.emplace_back() };
		sprite.TextureIndex = spriteIndex % s_TextureCount;
		sprite.SpriteTransform.Position = { horizontal(m_RandomEngine), vertical(m_RandomEngine), GetSortZ(sprite.TextureIndex) };
		sprite.SpriteTransform.Rotation = { 0.0f, 0.0f, rotation(m_RandomEngine) };
		sprite.SpriteTransform.Scale = { scale(m_RandomEngine), scale(m_RandomEngine), 1.0f };
		sprite.Velocity = { speed(m_RandomEngine), speed(m_RandomEngine) };
		sprite.SpinDegreesPerSecond = spin(m_RandomEngine);
	}

	float StressDemoState::GetSortZ(std::size_t textureIndex) const
	{
		return m_BatchingMode == BatchingMode::Sorted ? static_cast<float>(textureIndex) : 0.0f;
	}

	std::string_view StressDemoState::GetBatchingModeName() const
	{
		return m_BatchingMode == BatchingMode::Sorted ? "sorted" : "interleaved";
	}

	void StressDemoState::ApplyBatchingMode()
	{
		for (StressSprite& sprite : m_Sprites)
		{
			sprite.SpriteTransform.Position.z = GetSortZ(sprite.TextureIndex);
		}
	}

	void StressDemoState::MoveSprites(float seconds)
	{
		const BinaryEngine::Vector2f halfView{ m_Camera.GetWorldViewSize() * 0.5f };
		const BinaryEngine::Vector3f cameraPosition{ m_Camera.GetPosition() };
		const BinaryEngine::Vector2f minimum{ cameraPosition.x - halfView.x, cameraPosition.y - halfView.y };
		const BinaryEngine::Vector2f maximum{ cameraPosition.x + halfView.x, cameraPosition.y + halfView.y };

		for (StressSprite& sprite : m_Sprites)
		{
			BinaryEngine::Vector3f& position{ sprite.SpriteTransform.Position };
			position.x += sprite.Velocity.x * seconds;
			position.y += sprite.Velocity.y * seconds;

			if (position.x < minimum.x || position.x > maximum.x)
			{
				sprite.Velocity.x = -sprite.Velocity.x;
				position.x = std::clamp(position.x, minimum.x, maximum.x);
			}

			if (position.y < minimum.y || position.y > maximum.y)
			{
				sprite.Velocity.y = -sprite.Velocity.y;
				position.y = std::clamp(position.y, minimum.y, maximum.y);
			}

			sprite.SpriteTransform.Rotation.z += sprite.SpinDegreesPerSecond * seconds;
		}
	}

	void StressDemoState::UpdateHud()
	{
		const BinaryEngine::Renderer::Statistics& statistics{ m_Context.renderer.GetStatistics() };

		std::string text{ std::format("Sprites {}   Up / Down  10k   Left / Right  100k\n", m_Sprites.size()) };
		text += std::format("Batching {} (B)   Motion {} (Space)\n", GetBatchingModeName(), m_MotionEnabled ? "on" : "off");
		text += std::format("Quad capacity {} / {}", statistics.quadCapacity, statistics.maxQuads);

		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

	void StressDemoState::RecordFrame(float seconds)
	{
		if (m_WarmUpFramesLeft > 0)
		{
			m_WarmUpFramesLeft--;
			return;
		}

		m_MeasuredFrames++;
		m_MeasuredSeconds += seconds;
	}

	void StressDemoState::FinishMeasurement()
	{
		if (m_MeasuredFrames > 0)
		{
			const float averageMilliseconds{ 1000.0f * m_MeasuredSeconds / static_cast<float>(m_MeasuredFrames) };
			const BinaryEngine::Renderer::Statistics& statistics{ m_Context.renderer.GetStatistics() };
			APP_INFO("[StressDemoState] {} sprites, {}, motion {}: {} frames, average {:.2f} ms, {} quads, {} draw calls, capacity {}",
				m_Sprites.size(), GetBatchingModeName(), m_MotionEnabled ? "on" : "off", m_MeasuredFrames, averageMilliseconds, m_LastQuadCount, m_LastDrawCalls, statistics.quadCapacity);
		}

		m_MeasuredFrames = 0;
		m_MeasuredSeconds = 0.0f;
		m_WarmUpFramesLeft = s_WarmUpFrames;
	}

}
