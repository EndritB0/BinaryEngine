#pragma once

#include <string_view>

#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextureRegion.h>
#include <BinaryEngine/Scene/Scene.h>

namespace Sandbox {

	inline constexpr std::string_view s_PhysicsAtlasPath{ "./resources/physics/physics_atlas.png" };

	enum class AtlasSquare {
		Crate = 0,
		Scenery = 6
	};

	enum class AtlasDisc {
		Ball = 0
	};

	BinaryEngine::TextureRegion GetAtlasRegion(AtlasSquare square);
	BinaryEngine::TextureRegion GetAtlasRegion(AtlasDisc disc);
	BinaryEngine::Vector3f GetBoxSpriteScale(BinaryEngine::Vector2f size);
	BinaryEngine::Vector3f GetCircleSpriteScale(float radius);
	void DrawColliderOutlines(BinaryEngine::Scene& scene, BinaryEngine::Renderer& renderer);

}
