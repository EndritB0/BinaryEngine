#pragma once

#include <cstdint>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class RandomStream {
	public:
		static constexpr std::uint64_t s_DefaultSeed{ 42 };

		explicit RandomStream(std::uint64_t seed = s_DefaultSeed);

		float Float();
		float Float(float min, float max);
		std::int32_t Int(std::int32_t min, std::int32_t max);
		bool Chance(float probability);
		BinaryEngine::Vector2f Vector2f(BinaryEngine::Vector2f min, BinaryEngine::Vector2f max);

	private:
		std::uint32_t NextUInt32();
		std::uint32_t NextBounded(std::uint32_t bound);

	private:
		std::uint64_t m_State{ 0 };
		std::uint64_t m_Increment{ 0 };
	};

}