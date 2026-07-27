#pragma once

#include <string>

#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Renderer/TextureRegion.h"

namespace BinaryEngine {

	struct TagComponent {
		std::string Tag;

		TagComponent() : Tag(std::string()) {}
		TagComponent(const std::string& tag) : Tag(tag) {}
	};

	struct TransformComponent {
		Transform transform{ {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f} };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Transform& _transform) : transform(_transform) {}
	};

	struct SpriteComponent {
		AssetHandle TextureHandle{};
		TextureRegion Region{};
		bool UseRegion{ false };

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(AssetHandle handle) : TextureHandle(handle) {}
		SpriteComponent(AssetHandle handle, const TextureRegion& region) : TextureHandle(handle), Region(region), UseRegion(true) {}
	};

}