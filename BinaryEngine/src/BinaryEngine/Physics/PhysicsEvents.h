#pragma once

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	struct ContactBeginEvent {
		Entity FirstEntity;
		Entity SecondEntity;
	};

	struct ContactEndEvent {
		Entity FirstEntity;
		Entity SecondEntity;
	};

	struct ContactHitEvent {
		Entity FirstEntity;
		Entity SecondEntity;
		Vector2f Point{ 0.0f, 0.0f };
		Vector2f Normal{ 0.0f, 0.0f };
		float ApproachSpeed{ 0.0f };
	};

	struct TriggerBeginEvent {
		Entity TriggerEntity;
		Entity VisitorEntity;
	};

	struct TriggerEndEvent {
		Entity TriggerEntity;
		Entity VisitorEntity;
	};

}
