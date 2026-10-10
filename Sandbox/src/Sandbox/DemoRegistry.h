#pragma once
#include <span>
#include <string_view>

#include <BinaryEngine/State/StateManager.h>

namespace Sandbox {

	struct DemoEntry {
		std::string_view Name;
		void (*Push)(BinaryEngine::StateManager& stateManager);
	};

	std::span<const DemoEntry> GetDemos();
	const DemoEntry* FindDemo(std::string_view name);

}
