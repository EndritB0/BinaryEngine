#include "pch.h"
#include "BinaryEngine/Asset/AssetManager.h"

namespace BinaryEngine {

	AssetManager::AssetManager()
	{
		APP_TRACE("[AssetManager] Created");
	}

	AssetManager::~AssetManager()
	{
		ClearAssets();
		APP_TRACE("[AssetManager] Destroyed");
	}

	void AssetManager::UnloadAsset(AssetHandle handle)
	{
		auto assetIt = m_Assets.find(handle);

		if (assetIt == m_Assets.end())
		{
			CORE_WARN("[AssetManager] Attempted to unload an invalid or already unloaded Handle: {}", static_cast<std::uint64_t>(handle));
			return;
		}

		auto& asset = assetIt->second;

		std::string pathString{ asset->GetFilePath().string() };
		m_PathRegistry.erase(pathString);
		m_Assets.erase(assetIt);
		CORE_INFO("[AssetManager] Unloading Asset with Handle: {}, Path: {}", static_cast<std::uint64_t>(handle), pathString);
	}

	void AssetManager::CleanUnusedAssets()
	{
		bool assetRemoved{ false };

		for (auto it{ m_Assets.begin() }; it != m_Assets.end();)
		{
			if (it->second.use_count() == 1)
			{
				std::string pathString{ it->second->GetFilePath().string() };
				m_PathRegistry.erase(pathString);

				CORE_INFO("[AssetManager] Unloading Asset with Handle: {}, Path: {}", static_cast<std::uint64_t>(it->first), pathString);
				it = m_Assets.erase(it);

				assetRemoved = true;
			}
			else
			{
				++it;
			}
		}

		if (assetRemoved)
		{
			CORE_INFO("[AssetManager] Finished cleaning unused assets");
		}
	}

	void AssetManager::ClearAssets()
	{
		if (!m_Assets.empty())
		{
			m_PathRegistry.clear();
			m_Assets.clear();
			CORE_INFO("[AssetManager] Cleared all assets");
		}
	}

}