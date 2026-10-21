#pragma once

#include <cstdint>
#include <functional>

#include "BinaryEngine/Core/UUID.h"

namespace BinaryEngine {

	class AssetHandle {
	public:
		AssetHandle() = default;
		AssetHandle(std::uint64_t handle) :
			m_Handle(handle) {}

		static AssetHandle Generate() { return AssetHandle{ UUID::Generate() }; }

		bool IsValid() const { return m_Handle != 0; }
		operator std::uint64_t() const { return m_Handle; }
		bool operator==(const AssetHandle& other) const { return m_Handle == other.m_Handle; }
		bool operator!=(const AssetHandle& other) const { return m_Handle != other.m_Handle; }

	private:
		std::uint64_t m_Handle{};
	};

	enum class AssetType {
		None = 0,
		Texture2D,
		Shader,
		Font,
		AudioClip,
		StreamingAudioClip,
	};

	constexpr std::string_view AssetTypeToString(AssetType type)
	{
		switch (type)
		{
			case BinaryEngine::AssetType::None:
			{
				return "None";
			}

			case BinaryEngine::AssetType::Texture2D:
			{
				return "Texture2D";
			}

			case BinaryEngine::AssetType::Shader:
			{
				return "Shader";
			}

			case BinaryEngine::AssetType::Font:
			{
				return "Font";
			}

			case BinaryEngine::AssetType::AudioClip:
			{
				return "AudioClip";
			}

			case BinaryEngine::AssetType::StreamingAudioClip:
			{
				return "StreamingAudioClip";
			}

			default:
			{
				return "Unknown";
			}
		}
	}

}

namespace std {

	template<>
	struct hash<BinaryEngine::AssetHandle> {
		std::size_t operator()(const BinaryEngine::AssetHandle& handle) const
		{
			return std::hash<std::uint64_t>{}(static_cast<std::uint64_t>(handle));
		}
	};

}

template<>
struct fmt::formatter<BinaryEngine::AssetHandle> {
	constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
	{
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const BinaryEngine::AssetHandle& handle, FormatContext& ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "{}", static_cast<uint64_t>(handle));
	}
};