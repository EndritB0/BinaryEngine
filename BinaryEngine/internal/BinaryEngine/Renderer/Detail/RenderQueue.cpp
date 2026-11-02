#include "pch.h"
#include "BinaryEngine/Renderer/Detail/RenderQueue.h"

namespace {

	constexpr std::uint64_t s_ScreenSpaceKeyBit{ std::uint64_t{ 1 } << 63 };
	constexpr std::uint32_t s_SortKeyIndexBits{ 31 };
	constexpr std::uint64_t s_SortKeyIndexMask{ (std::uint64_t{ 1 } << s_SortKeyIndexBits) - 1 };

	std::uint32_t ToOrderableBits(float value)
	{
		const float normalisedValue{ value == 0.0f ? 0.0f : value };
		const std::uint32_t bits{ std::bit_cast<std::uint32_t>(normalisedValue) };
		return (bits & 0x80000000u) != 0 ? ~bits : (bits | 0x80000000u);
	}

	std::uint64_t MakeSortKey(BinaryEngine::Detail::QuadSpace space, float sortZ, std::uint32_t sceneEntryIndex)
	{
		const std::uint64_t spaceBit{ space == BinaryEngine::Detail::QuadSpace::Screen ? s_ScreenSpaceKeyBit : 0 };
		return spaceBit | (static_cast<std::uint64_t>(ToOrderableBits(sortZ)) << s_SortKeyIndexBits) | sceneEntryIndex;
	}

}

namespace BinaryEngine::Detail {

	void RenderQueue::Initialise(std::uint32_t initialEntryCapacity, std::uint32_t maxEntryCount)
	{
		m_MaxEntryCount = maxEntryCount;

		m_Entries.reserve(initialEntryCapacity);
		m_SortKeys.reserve(initialEntryCapacity);
		for (std::vector<std::uint32_t>& drawOrder : m_DrawOrders)
		{
			drawOrder.reserve(initialEntryCapacity);
		}
		m_Batches.reserve(64);
		m_Scenes.reserve(4);
	}

	void RenderQueue::ResetFrame()
	{
		m_Entries.clear();
		for (std::vector<std::uint32_t>& drawOrder : m_DrawOrders)
		{
			drawOrder.clear();
		}
		m_Batches.clear();
		m_Scenes.clear();
		m_SceneOpen = false;
		m_KeptEntryCount = 0;

		if (m_FrameDroppedEntryCount == 0)
		{
			m_DropWarned = false;
		}
		m_FrameDroppedEntryCount = 0;
	}

	void RenderQueue::OpenScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& screenViewProjectionMatrix, const CameraViewport& viewport)
	{
		SceneRecord& scene{ m_Scenes.emplace_back() };
		scene.ViewProjectionMatrix = viewProjectionMatrix;
		scene.ScreenViewProjectionMatrix = screenViewProjectionMatrix;
		scene.Viewport = viewport;
		scene.FirstEntry = m_Entries.size();
		m_SceneOpen = true;
	}

	const SceneRecord& RenderQueue::CloseScene()
	{
		m_SceneOpen = false;

		SceneRecord& scene{ m_Scenes.back() };
		const std::uint32_t sceneEntryCount{ static_cast<std::uint32_t>(m_Entries.size() - scene.FirstEntry) };

		m_SortKeys.clear();
		for (std::uint32_t sceneEntryIndex{ 0 }; sceneEntryIndex < sceneEntryCount; sceneEntryIndex++)
		{
			const RenderQueueEntry& entry{ m_Entries[scene.FirstEntry + sceneEntryIndex] };
			m_SortKeys.push_back(MakeSortKey(entry.Space, entry.SortZ, sceneEntryIndex));
		}
		std::sort(m_SortKeys.begin(), m_SortKeys.end());

		const auto firstScreenKey{ std::partition_point(m_SortKeys.begin(), m_SortKeys.end(), [](std::uint64_t sortKey) { return (sortKey & s_ScreenSpaceKeyBit) == 0; }) };
		const std::uint32_t worldEntryCount{ static_cast<std::uint32_t>(firstScreenKey - m_SortKeys.begin()) };
		const std::uint32_t screenEntryCount{ sceneEntryCount - worldEntryCount };

		const std::uint32_t entryBudget{ m_MaxEntryCount - m_KeptEntryCount };
		const std::uint32_t keptScreenEntryCount{ std::min(screenEntryCount, entryBudget) };
		const std::uint32_t keptWorldEntryCount{ std::min(worldEntryCount, entryBudget - keptScreenEntryCount) };
		m_FrameDroppedEntryCount += sceneEntryCount - keptWorldEntryCount - keptScreenEntryCount;

		scene.FirstBatch = m_Batches.size();
		scene.WorldBatchCount = AppendBatches(scene, 0, keptWorldEntryCount);
		scene.ScreenBatchCount = AppendBatches(scene, worldEntryCount, keptScreenEntryCount);
		scene.KeptEntryCount = keptWorldEntryCount + keptScreenEntryCount;

		m_KeptEntryCount += scene.KeptEntryCount;
		return scene;
	}

	void RenderQueue::DiscardOpenScene()
	{
		m_Entries.resize(m_Scenes.back().FirstEntry);
		m_Scenes.pop_back();
		m_SceneOpen = false;
	}

	bool RenderQueue::ConsumeDropWarning()
	{
		if (m_FrameDroppedEntryCount == 0 || m_DropWarned)
		{
			return false;
		}

		m_DropWarned = true;
		return true;
	}

	std::size_t RenderQueue::AppendBatches(const SceneRecord& scene, std::uint32_t firstKeyIndex, std::uint32_t keyCount)
	{
		const std::size_t firstBatchIndex{ m_Batches.size() };

		for (std::uint32_t keyIndex{ firstKeyIndex }; keyIndex < firstKeyIndex + keyCount; keyIndex++)
		{
			const RenderQueueEntry& entry{ m_Entries[scene.FirstEntry + (m_SortKeys[keyIndex] & s_SortKeyIndexMask)] };
			std::vector<std::uint32_t>& drawOrder{ m_DrawOrders[static_cast<std::size_t>(entry.RendererType)] };

			const bool startsNewBatch{ m_Batches.size() == firstBatchIndex
				|| m_Batches.back().RendererType != entry.RendererType
				|| m_Batches.back().State != entry.State
				|| m_Batches.back().Texture != entry.Texture };

			if (startsNewBatch)
			{
				m_Batches.emplace_back(entry.State, entry.Texture, entry.RendererType, static_cast<std::uint32_t>(drawOrder.size()), 0);
			}

			drawOrder.push_back(entry.CommandIndex);
			m_Batches.back().ItemCount++;
		}

		return m_Batches.size() - firstBatchIndex;
	}

}
