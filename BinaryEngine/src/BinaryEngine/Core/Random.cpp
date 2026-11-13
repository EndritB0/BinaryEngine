#include "pch.h"
#include "BinaryEngine/Core/Random.h"

#include "BinaryEngine/Core/RandomStream.h"

namespace {

	std::uint64_t GenerateRandomSeed()
	{
		std::random_device randomDevice;
		std::uniform_int_distribution<std::uint64_t> seedDistribution;
		return seedDistribution(randomDevice);
	}

	BinaryEngine::RandomStream& GetGlobalStream()
	{
		static BinaryEngine::RandomStream s_GlobalStream{ GenerateRandomSeed() };
		return s_GlobalStream;
	}

}

namespace BinaryEngine {

	float Random::Float()
	{
		return GetGlobalStream().Float();
	}

	float Random::Float(float min, float max)
	{
		return GetGlobalStream().Float(min, max);
	}

	std::int32_t Random::Int(std::int32_t min, std::int32_t max)
	{
		return GetGlobalStream().Int(min, max);
	}

	bool Random::Chance(float probability)
	{
		return GetGlobalStream().Chance(probability);
	}

	BinaryEngine::Vector2f Random::Vector2f(BinaryEngine::Vector2f min, BinaryEngine::Vector2f max)
	{
		return GetGlobalStream().Vector2f(min, max);
	}

}