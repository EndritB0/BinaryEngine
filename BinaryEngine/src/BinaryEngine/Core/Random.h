#pragma once

#include <cstdint>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class Random {
	public:
		Random() = delete;
		~Random() = delete;
		Random(const Random&) = delete;
		Random& operator=(const Random&) = delete;
		Random(Random&&) = delete;
		Random& operator=(Random&&) = delete;

		static float Float();
		static float Float(float min, float max);
		static std::int32_t Int(std::int32_t min, std::int32_t max);
		static bool Chance(float probability);
		static BinaryEngine::Vector2f Vector2f(BinaryEngine::Vector2f min, BinaryEngine::Vector2f max);
	};

}