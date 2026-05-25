#pragma once

#include <string>

#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Asset/AssetTypes.h"

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

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(AssetHandle handle) : TextureHandle(handle) {}
	};

}