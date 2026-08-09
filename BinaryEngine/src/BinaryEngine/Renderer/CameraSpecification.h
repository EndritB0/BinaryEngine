#pragma once

#include <optional>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	enum class CameraViewportMode {
		Expand,
		FixedHeight,
		FixedWidth,
		Letterbox,
		Stretch
	};

	enum class CameraSnapMode {
		None,
		SnapCamera,
		PixelPerfect
	};

	struct CameraViewport {
		Vector2i Position{ 0, 0 };
		Vector2i Size{ 0, 0 };
	};

	struct CameraSpecification {
		std::optional<Vector2f> DesignSize{};
		CameraViewportMode ViewportMode{ CameraViewportMode::FixedHeight };
		CameraSnapMode SnapMode{ CameraSnapMode::None };
	};

}
