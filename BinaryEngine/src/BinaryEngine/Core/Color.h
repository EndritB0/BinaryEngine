#pragma once

#include <cstdint>

namespace BinaryEngine {

	struct Color {
		std::uint8_t red{ 0 };
		std::uint8_t green{ 0 };
		std::uint8_t blue{ 0 };
		std::uint8_t alpha{ 0xFF };

		constexpr Color() = default;
		constexpr Color(std::uint8_t _red, std::uint8_t _green, std::uint8_t _blue, std::uint8_t _alpha = 0xFF)
			: red(_red), green(_green), blue(_blue), alpha(_alpha)
		{
		}

		constexpr Color(std::uint32_t rgba)
		{
			red = (rgba >> 24) & 0xFF;
			green = (rgba >> 16) & 0xFF;
			blue = (rgba >> 8) & 0xFF;
			alpha = rgba & 0xFF;
		}

		constexpr bool operator==(const Color& other) const
		{
			return this->red == other.red
				&& this->green == other.green
				&& this->blue == other.blue
				&& this->alpha == other.alpha;
		}

		constexpr bool operator!=(const Color& other) const
		{
			return !(*this == other);
		}

		static const Color Black;
		static const Color White;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Transparent;
	};

	inline const Color Color::Black{ 0x00, 0x00, 0x00 };
	inline const Color Color::White{ 0xFF, 0xFF, 0xFF };
	inline const Color Color::Red{ 0xFF, 0x00, 0x00 };
	inline const Color Color::Green{ 0x00, 0xFF, 0x00 };
	inline const Color Color::Blue{ 0x00, 0x00, 0xFF };
	inline const Color Color::Transparent{ 0x00, 0x00, 0x00, 0x00 };

}