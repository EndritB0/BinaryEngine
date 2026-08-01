#pragma once

#include <optional>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/OrthographicCamera.h"

namespace BinaryEngine {

	struct CameraSpecification {
		std::optional<Vector2f> DesignSize{};
		CameraViewportMode ViewportMode{ CameraViewportMode::FixedHeight };
	};

}
