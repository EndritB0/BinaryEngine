#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "BinaryEngine/Renderer/CameraSpecification.h"
#include "BinaryEngine/Renderer/Detail/BatchRendererType.h"
#include "BinaryEngine/Renderer/RenderSpace.h"

struct SDL_GPUTexture;

namespace BinaryEngine::Detail {

	struct RenderState;

	inline constexpr std::uint32_t s_AbsoluteMaxQuads{ 16000000 };

	struct RenderQueueEntry {
		const RenderState* State{ nullptr };
		::SDL_GPUTexture* Texture{ nullptr };
		std::uint32_t CommandIndex{ 0 };
		float SortZ{ 0.0f };
		BatchRendererType RendererType{ BatchRendererType::Sprite };
		RenderSpace Space{ RenderSpace::World };
	};

	struct RenderBatch {
		const RenderState* State{ nullptr };
		::SDL_GPUTexture* Texture{ nullptr };
		BatchRendererType RendererType{ BatchRendererType::Sprite };
		std::uint32_t FirstItem{ 0 };
		std::uint32_t ItemCount{ 0 };
	};

	struct SceneRecord {
		glm::mat4 ViewProjectionMatrix{ 1.0f };
		glm::mat4 ScreenViewProjectionMatrix{ 1.0f };
		CameraViewport Viewport{};
		std::size_t FirstEntry{ 0 };
		std::size_t FirstBatch{ 0 };
		std::size_t WorldBatchCount{ 0 };
		std::size_t ScreenBatchCount{ 0 };
		std::uint32_t KeptEntryCount{ 0 };
	};

	class RenderQueue {
	public:
		void Initialise(std::uint32_t initialEntryCapacity, std::uint32_t maxEntryCount);
		void ResetFrame();

		void OpenScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& screenViewProjectionMatrix, const CameraViewport& viewport);
		const SceneRecord& CloseScene();
		void DiscardOpenScene();
		bool IsSceneOpen() const { return m_SceneOpen; }

		bool HasRoomForEntry()
		{
			if (m_Entries.size() - m_Scenes.back().FirstEntry < s_AbsoluteMaxQuads)
			{
				return true;
			}

			m_FrameDroppedEntryCount++;
			return false;
		}

		void AddEntry(const RenderState* state, ::SDL_GPUTexture* texture, std::uint32_t commandIndex, float sortZ, BatchRendererType rendererType, RenderSpace space)
		{
			m_Entries.emplace_back(state, texture, commandIndex, sortZ, rendererType, space);
		}

		bool ConsumeDropWarning();
		std::uint32_t GetFrameDroppedEntryCount() const { return m_FrameDroppedEntryCount; }
		std::uint32_t GetKeptEntryCount() const { return m_KeptEntryCount; }
		const std::vector<std::uint32_t>& GetDrawOrder(std::size_t rendererIndex) const { return m_DrawOrders[rendererIndex]; }
		const std::vector<SceneRecord>& GetScenes() const { return m_Scenes; }
		const RenderBatch& GetBatch(std::size_t batchIndex) const { return m_Batches[batchIndex]; }

	private:
		std::size_t AppendBatches(const SceneRecord& scene, std::uint32_t firstKeyIndex, std::uint32_t keyCount);

	private:
		std::vector<RenderQueueEntry> m_Entries;
		std::vector<std::uint64_t> m_SortKeys;
		std::array<std::vector<std::uint32_t>, s_BatchRendererCount> m_DrawOrders;
		std::vector<RenderBatch> m_Batches;
		std::vector<SceneRecord> m_Scenes;
		std::uint32_t m_MaxEntryCount{ 0 };
		std::uint32_t m_KeptEntryCount{ 0 };
		std::uint32_t m_FrameDroppedEntryCount{ 0 };
		bool m_SceneOpen{ false };
		bool m_DropWarned{ false };
	};

}
