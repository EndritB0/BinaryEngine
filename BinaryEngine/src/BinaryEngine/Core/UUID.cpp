#include "pch.h"
#include "BinaryEngine/Core/UUID.h"

namespace BinaryEngine {

	namespace {
		std::random_device s_RandomDevice;
		std::mt19937 s_UUIDGenerator(s_RandomDevice());
		std::uniform_int_distribution<std::uint64_t> s_Distribution;
	}

	std::uint64_t UUID::Generate()
	{
		return s_Distribution(s_UUIDGenerator);
	}
}