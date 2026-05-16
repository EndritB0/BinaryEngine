#pragma once

#include <cstdint>

namespace BinaryEngine {

	using MouseCode = std::uint8_t;

	namespace Mouse {

		inline constexpr MouseCode Unknown{ 0 };
		inline constexpr MouseCode Button1{ 1 };
		inline constexpr MouseCode Button2{ 3 };
		inline constexpr MouseCode Button3{ 2 };
		inline constexpr MouseCode Button4{ 4 };
		inline constexpr MouseCode Button5{ 5 };

		inline constexpr MouseCode Left{ Button1 };
		inline constexpr MouseCode Right{ Button2 };
		inline constexpr MouseCode Middle{ Button3 };
		inline constexpr MouseCode Side1{ Button4 };
		inline constexpr MouseCode Side2{ Button5 };
	}

}