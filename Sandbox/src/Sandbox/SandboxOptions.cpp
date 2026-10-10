#include "pch.h"
#include "Sandbox/SandboxOptions.h"

#include "Sandbox/DemoRegistry.h"

namespace Sandbox {

	namespace {

		constexpr std::string_view s_Usage{ "Usage: Sandbox [--frames N] [--state <Name> | --all-states]" };

		std::string ListDemoNames()
		{
			std::string names;
			for (const DemoEntry& demo : GetDemos())
			{
				if (!names.empty())
				{
					names += ", ";
				}
				names += demo.Name;
			}
			return names;
		}

		std::optional<std::uint32_t> ParseFrameCount(std::string_view text)
		{
			std::uint32_t frameCount{ 0 };
			const char* textEnd{ text.data() + text.size() };
			const auto [parseEnd, parseError]{ std::from_chars(text.data(), textEnd, frameCount) };

			if (parseError != std::errc{} || parseEnd != textEnd || frameCount == 0)
			{
				return std::nullopt;
			}
			return frameCount;
		}

	}

	std::optional<SandboxOptions> ParseSandboxOptions(int argc, char* argv[])
	{
		const std::span<char*> arguments{ argv, static_cast<std::size_t>(argc) };
		SandboxOptions options;

		for (std::size_t argumentIndex{ 1 }; argumentIndex < arguments.size(); ++argumentIndex)
		{
			const std::string_view argument{ arguments[argumentIndex] };
			const bool hasValue{ argumentIndex + 1 < arguments.size() };

			if (argument == "--frames")
			{
				if (!hasValue)
				{
					APP_ERROR("[Sandbox] --frames needs a frame count, e.g. --frames 300. {}", s_Usage);
					return std::nullopt;
				}

				const std::string_view value{ arguments[argumentIndex + 1] };
				++argumentIndex;
				options.Frames = ParseFrameCount(value);
				if (!options.Frames.has_value())
				{
					APP_ERROR("[Sandbox] --frames needs a whole number of at least 1, got '{}'", value);
					return std::nullopt;
				}
			}
			else if (argument == "--state")
			{
				if (!hasValue)
				{
					APP_ERROR("[Sandbox] --state needs a demo name. Demos: {}", ListDemoNames());
					return std::nullopt;
				}

				const std::string_view value{ arguments[argumentIndex + 1] };
				++argumentIndex;
				if (FindDemo(value) == nullptr)
				{
					APP_ERROR("[Sandbox] Unknown demo '{}'. Demos: {}", value, ListDemoNames());
					return std::nullopt;
				}
				options.StartDemo = value;
			}
			else if (argument == "--all-states")
			{
				options.AllDemos = true;
			}
			else
			{
				APP_ERROR("[Sandbox] Unknown argument '{}'. {}", argument, s_Usage);
				return std::nullopt;
			}
		}

		if (options.AllDemos && !options.Frames.has_value())
		{
			APP_ERROR("[Sandbox] --all-states needs --frames N, the number of frames each demo runs");
			return std::nullopt;
		}

		if (options.AllDemos && !options.StartDemo.empty())
		{
			APP_ERROR("[Sandbox] --state and --all-states can't be combined");
			return std::nullopt;
		}

		return options;
	}

}
