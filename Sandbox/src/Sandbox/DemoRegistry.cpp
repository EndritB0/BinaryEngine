#include "pch.h"
#include "Sandbox/DemoRegistry.h"

#include <BinaryEngine/State/StateManager.h>

#include "Sandbox/States/ShowcaseDemoState.h"

namespace Sandbox {

	namespace {

		const DemoEntry s_Demos[]{
			{ "Showcase", [](BinaryEngine::StateManager& stateManager) { stateManager.RequestPushState<ShowcaseDemoState>(); } },
		};

		static_assert(std::size(s_Demos) <= 12, "The shell selects demos with F1-F12, so there can be at most 12");

	}

	std::span<const DemoEntry> GetDemos()
	{
		return s_Demos;
	}

	const DemoEntry* FindDemo(std::string_view name)
	{
		for (const DemoEntry& demo : s_Demos)
		{
			if (demo.Name == name)
			{
				return &demo;
			}
		}

		return nullptr;
	}

}
