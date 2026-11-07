#include "pch.h"
#include "BinaryEngine/Physics/Detail/PhysicsGlobals.h"

#include <box2d/box2d.h>

namespace BinaryEngine::Detail {

	namespace {

		int OnBox2DAssert(const char* condition, const char* fileName, int lineNumber)
		{
			CORE_ERROR("[Physics] Box2D assertion failed: {} ({}:{})", condition, fileName, lineNumber);
			return 1;
		}

		void OnBox2DLog(const char* message)
		{
			std::string_view text{ message };
			while (!text.empty() && text.back() == '\n')
			{
				text.remove_suffix(1);
			}

			CORE_WARN("[Physics] Box2D: {}", text);
		}

	}

	void ConfigurePhysics(const PhysicsSpecification& specification)
	{
		float worldUnitsPerMeter{ specification.WorldUnitsPerMeter };
		if (!std::isfinite(worldUnitsPerMeter) || worldUnitsPerMeter <= 0.0f)
		{
			const float defaultWorldUnitsPerMeter{ PhysicsSpecification{}.WorldUnitsPerMeter };
			CORE_WARN("[Physics] WorldUnitsPerMeter is {}, which isn't a finite number above 0. Using {}", worldUnitsPerMeter, defaultWorldUnitsPerMeter);
			worldUnitsPerMeter = defaultWorldUnitsPerMeter;
		}

		b2SetLengthUnitsPerMeter(worldUnitsPerMeter);
		b2SetAssertCallback(OnBox2DAssert);
		b2SetLogCallback(OnBox2DLog);

		const b2Version version{ b2GetVersion() };
		CORE_INFO("[Physics] Box2D {}.{}.{} configured: {} world units per metre", version.major, version.minor, version.revision, worldUnitsPerMeter);
	}

}
