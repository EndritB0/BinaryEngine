#pragma once

#include <filesystem>

#include "BinaryEngine/Asset/AssetTypes.h"

namespace BinaryEngine {

	class Asset {
	public:
		virtual ~Asset() = default;

		virtual AssetType GetAssetType() const = 0;
		AssetHandle GetHandle() const { return m_Handle; }
		const std::filesystem::path& GetFilePath() const { return m_FilePath; }

	private:
		friend class AssetManager;

		AssetHandle m_Handle{};
		std::filesystem::path m_FilePath;
	};

}