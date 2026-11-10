#pragma once

#include <span>
#include <string_view>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Audio/AudioScope.h>
#include <BinaryEngine/Audio/AudioTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Physics/PhysicsEvents.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextureRegion.h>
#include <BinaryEngine/Scene/Scene.h>

namespace Sandbox {

	inline constexpr std::string_view s_PhysicsAtlasPath{ "./resources/physics/physics_atlas.png" };

	enum class AtlasSquare {
		Crate = 0,
		IcyCrate,
		HeavyCrate,
		UprightCrate,
		Debris,
		Pane,
		Scenery,
		Paddle,
		Bucket,
		TableWall
	};

	enum class AtlasDisc {
		Ball = 0,
		BouncyBall,
		Balloon,
		Shot,
		CuePuck,
		RedPuck,
		YellowPuck,
		Pocket
	};

	BinaryEngine::TextureRegion GetAtlasRegion(AtlasSquare square);
	BinaryEngine::TextureRegion GetAtlasRegion(AtlasDisc disc);
	BinaryEngine::Vector3f GetBoxSpriteScale(BinaryEngine::Vector2f size);
	BinaryEngine::Vector3f GetCircleSpriteScale(float radius);
	void DrawColliderOutlines(BinaryEngine::Scene& scene, BinaryEngine::Renderer& renderer);
	void PlayHitSounds(BinaryEngine::AudioScope& audio, std::span<const BinaryEngine::ContactHitEvent> hitEvents, BinaryEngine::AssetHandle sound, BinaryEngine::AudioBusHandle bus);

}
