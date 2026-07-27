#pragma once

#include <vector>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/TextureRegion.h"

namespace BinaryEngine {

	struct AnimationClip {
		std::vector<TextureRegion> Frames;
		float FrameDuration{ 0.1f };
		bool Looping{ true };
	};

	AnimationClip MakeStripClip(Vector2i origin, Vector2i frameSize, int count, int columns, float frameDuration, bool looping = true)
	{
		AnimationClip clip;
		clip.FrameDuration = frameDuration;
		clip.Looping = looping;
		clip.Frames.reserve(static_cast<std::size_t>(count > 0 ? count : 0));

		const int safeColumns{ columns > 0 ? columns : 1 };
		for (int i{ 0 }; i < count; i++)
		{
			const int col{ i % safeColumns };
			const int row{ i / safeColumns };
			TextureRegion& region{ clip.Frames.emplace_back() };
			region.Position.x = origin.x + col * frameSize.x;
			region.Position.y = origin.y + row * frameSize.y;
			region.Size = frameSize;
		}
		return clip;
	}

}
