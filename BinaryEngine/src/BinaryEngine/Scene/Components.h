#pragma once

#include <string>

#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Renderer/AnimationClip.h"
#include "BinaryEngine/Renderer/TextSpecification.h"
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
		TransformComponent(const Transform& initialTransform) : transform(initialTransform) {}
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

	struct TextComponent {
		AssetHandle FontHandle{};
		std::string Text;
		TextSpecification Specification{};

		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
		TextComponent(AssetHandle handle, const std::string& text) : FontHandle(handle), Text(text) {}
		TextComponent(AssetHandle handle, const std::string& text, const TextSpecification& specification) : FontHandle(handle), Text(text), Specification(specification) {}
	};

	struct AnimationComponent {
		const AnimationClip* CurrentClip{ nullptr };
		float ElapsedTime{ 0.0f };
		std::size_t CurrentFrame{ 0 };
		bool Playing{ true };
		bool Finished{ false };

		AnimationComponent() = default;
		AnimationComponent(const AnimationComponent&) = default;
		AnimationComponent(const AnimationClip* clip) : CurrentClip(clip) {}

		void Play(const AnimationClip* clip)
		{
			if (CurrentClip == clip)
			{
				return;
			}

			CurrentClip = clip;
			ElapsedTime = 0.0f;
			CurrentFrame = 0;
			Playing = true;
			Finished = false;
		}
	};

}