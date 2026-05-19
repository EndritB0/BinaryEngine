#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "BinaryEngine/Asset/Asset.h"
#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Log/Log.h"

namespace BinaryEngine {

	class AssetManager {
	public:
		AssetManager();
		~AssetManager();

		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		template<typename T, typename... Args>
			requires(std::is_base_of_v<Asset, T>)
		AssetHandle LoadAsset(const std::filesystem::path& path, Args&&... args)
		{
			std::string pathString{ path.string() };

			if (m_PathRegistry.find(pathString) != m_PathRegistry.end())
			{
				CORE_TRACE("[AssetManager] Asset Already Loaded, returning existing handle");
				return m_PathRegistry[pathString];
			}

			AssetHandle handle;
			std::shared_ptr<T> asset = std::make_shared<T>();

			asset->Load(path, std::forward<Args>(args)...);
			asset->m_Handle = handle;
			asset->m_FilePath = path;

			m_PathRegistry[pathString] = handle;
			m_Assets[handle] = asset;
			CORE_INFO("[AssetManager] Successfully loaded Asset: {}, Handle: {}", pathString, static_cast<std::uint64_t>(handle));
			return handle;
		}

		template<typename T>
			requires(std::is_base_of_v<Asset, T>)
		std::shared_ptr<T> GetAsset(AssetHandle handle) const
		{
			if (!handle.IsValid())
			{
				CORE_ERROR("[AssetManager] Invalid Asset Handle: {}", static_cast<std::uint64_t>(handle));
				return nullptr;
			}

			auto assetIt = m_Assets.find(handle);

			if (assetIt == m_Assets.end())
			{
				CORE_ERROR("[AssetManager] Asset with Handle {} not found", static_cast<std::uint64_t>(handle));
				return nullptr;
			}

			std::shared_ptr<T> asset = std::dynamic_pointer_cast<T>(assetIt->second);

			if (asset)
			{
				return asset;
			}

			CORE_ERROR("[AssetManager] Asset ID : {} Asset Type Mismatch", static_cast<std::uint64_t>(handle));
			return nullptr;

		}

		void UnloadAsset(AssetHandle handle);
		void CleanUnusedAssets();
		void ClearAssets();

	private:
		std::unordered_map < std::string, AssetHandle> m_PathRegistry;
		std::unordered_map < AssetHandle, std::shared_ptr<Asset>> m_Assets;
	};

}