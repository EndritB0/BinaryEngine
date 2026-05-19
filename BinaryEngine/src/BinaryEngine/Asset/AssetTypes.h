#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "BinaryEngine/Core/UUID.h"

namespace BinaryEngine {

	class AssetHandle {
	public:
		AssetHandle() : m_Handle(UUID::Generate()) {}
		AssetHandle(std::uint64_t handle) : m_Handle(handle) {}

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
	};

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