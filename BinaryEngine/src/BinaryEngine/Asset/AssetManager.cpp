#include "pch.h"
#include "BinaryEngine/Asset/AssetManager.h"

namespace BinaryEngine {

	AssetManager::AssetManager()
	{
		CORE_INFO("[AssetManager] Asset Manager Initialised");
	}

	AssetManager::~AssetManager()
	{
		ClearAssets();
		CORE_INFO("[AssetManager] Asset Manager Shutdown");
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
		CORE_INFO("[AssetManager] Unloaded Asset with Handle: {}, Path: {}", handle, pathString);
	}

	void AssetManager::CleanUnusedAssets()
	{
		bool assetRemoved{ false };

		for (auto it{ m_Assets.begin() }; it != m_Assets.end();)
		{
			auto& [handle, asset] = *it;
			if (asset.use_count() == 1)
			{
				std::string pathString{ asset->GetFilePath().string() };
				m_PathRegistry.erase(pathString);

				it = m_Assets.erase(it);
				CORE_INFO("[AssetManager] Unloading Asset with Handle: {}, Path: {}", handle, pathString);

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
		CORE_TRACE("[AssetManager] Request to clear all Assets");

		if (!m_Assets.empty())
		{
			m_PathRegistry.clear();
			m_Assets.clear();
			CORE_INFO("[AssetManager] All assets cleared successfully");
		}
	}

}