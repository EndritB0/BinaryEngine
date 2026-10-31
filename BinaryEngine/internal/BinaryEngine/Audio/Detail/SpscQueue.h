#pragma once

#include <algorithm>
#include <atomic>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace BinaryEngine::Detail {

	template<typename T>
		requires(std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>)
	class SpscQueue {
	public:
		explicit SpscQueue(std::uint32_t minimumCapacity) :
			m_Elements(std::bit_ceil(std::max(minimumCapacity, 1u))), m_IndexMask(static_cast<std::uint32_t>(m_Elements.size()) - 1) {}

		SpscQueue(const SpscQueue&) = delete;
		SpscQueue& operator=(const SpscQueue&) = delete;
		SpscQueue(SpscQueue&&) = delete;
		SpscQueue& operator=(SpscQueue&&) = delete;

		bool TryPush(const T& element)
		{
			const std::uint32_t tail{ m_Tail.Index.load(std::memory_order_relaxed) };
			const std::uint32_t head{ m_Head.Index.load(std::memory_order_acquire) };
			if (tail - head > m_IndexMask)
			{
				return false;
			}

			m_Elements[tail & m_IndexMask] = element;
			m_Tail.Index.store(tail + 1, std::memory_order_release);
			return true;
		}

		bool TryPop(T& element)
		{
			const std::uint32_t head{ m_Head.Index.load(std::memory_order_relaxed) };
			const std::uint32_t tail{ m_Tail.Index.load(std::memory_order_acquire) };
			if (head == tail)
			{
				return false;
			}

			element = m_Elements[head & m_IndexMask];
			m_Head.Index.store(head + 1, std::memory_order_release);
			return true;
		}

	private:
		static constexpr std::size_t s_CacheLineSize{ 64 };

		struct PaddedIndex {
			std::byte LeadingPadding[s_CacheLineSize]{};
			std::atomic<std::uint32_t> Index{ 0 };
			std::byte TrailingPadding[s_CacheLineSize - sizeof(std::atomic<std::uint32_t>)]{};
		};

		std::vector<T> m_Elements;
		std::uint32_t m_IndexMask{ 0 };
		PaddedIndex m_Head;
		PaddedIndex m_Tail;
	};

}
