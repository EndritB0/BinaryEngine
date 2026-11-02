#pragma once

#include <cstddef>
#include <cstdint>

namespace BinaryEngine::Detail {

	enum class BatchRendererType : std::uint8_t {
		Sprite = 0,
		Shape,
		Count
	};

	inline constexpr std::size_t s_BatchRendererCount{ static_cast<std::size_t>(BatchRendererType::Count) };

}
