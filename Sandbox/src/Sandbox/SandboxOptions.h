#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace Sandbox {

	struct SandboxOptions {
		std::optional<std::uint32_t> Frames;
		std::string StartDemo;
		bool AllDemos{ false };
	};

	std::optional<SandboxOptions> ParseSandboxOptions(int argc, char* argv[]);

}
