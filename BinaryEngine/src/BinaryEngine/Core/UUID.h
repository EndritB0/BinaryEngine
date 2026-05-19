#pragma once

#include <cstdint>

namespace BinaryEngine {

	class UUID {
	public:
		static std::uint64_t Generate();
	};

}