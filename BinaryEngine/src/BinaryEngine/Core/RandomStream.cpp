/*
 * Changed: pcg32_srandom_r, pcg32_random_r and pcg32_boundedrand_r rewritten in C++ as RandomStream
 * Source: pcg_basic.c
 * URL: https://github.com/imneme/pcg-c-basic
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */

#include "pch.h"
#include "BinaryEngine/Core/RandomStream.h"

namespace {

	constexpr std::uint64_t s_Multiplier{ 6364136223846793005 };
	constexpr std::uint64_t s_StreamSelector{ 54 };

}

namespace BinaryEngine {

	RandomStream::RandomStream(std::uint64_t seed) :
		m_Increment((s_StreamSelector << 1) | 1)
	{
		NextUInt32();
		m_State += seed;
		NextUInt32();
	}

	float RandomStream::Float()
	{
		const std::uint32_t topBits{ NextUInt32() >> 8 };
		return static_cast<float>(topBits) * 0x1.0p-24f;
	}

	float RandomStream::Float(float min, float max)
	{
		return std::fma(max - min, Float(), min);
	}

	std::int32_t RandomStream::Int(std::int32_t min, std::int32_t max)
	{
		if (min > max)
		{
			std::swap(min, max);
		}

		const std::uint32_t range{ static_cast<std::uint32_t>(max) - static_cast<std::uint32_t>(min) + 1u };
		const std::uint32_t offset{ range == 0 ? NextUInt32() : NextBounded(range) };
		return static_cast<std::int32_t>(static_cast<std::uint32_t>(min) + offset);
	}

	bool RandomStream::Chance(float probability)
	{
		return Float() < probability;
	}

	BinaryEngine::Vector2f RandomStream::Vector2f(BinaryEngine::Vector2f min, BinaryEngine::Vector2f max)
	{
		return BinaryEngine::Vector2f{ Float(min.x, max.x), Float(min.y, max.y) };
	}

	std::uint32_t RandomStream::NextUInt32()
	{
		const std::uint64_t oldState{ m_State };
		m_State = oldState * s_Multiplier + m_Increment;

		const std::uint32_t xorShifted{ static_cast<std::uint32_t>(((oldState >> 18) ^ oldState) >> 27) };
		const int rotation{ static_cast<int>(oldState >> 59) };
		return std::rotr(xorShifted, rotation);
	}

	std::uint32_t RandomStream::NextBounded(std::uint32_t bound)
	{
		const std::uint32_t threshold{ (0u - bound) % bound };
		std::uint32_t raw{ NextUInt32() };
		while (raw < threshold)
		{
			raw = NextUInt32();
		}
		return raw % bound;
	}

}