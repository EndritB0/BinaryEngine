#include "pch.h"
#include "BinaryEngine/Core/Assert.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace {

	std::string_view GetFileName(std::string_view filePath)
	{
		const std::size_t separatorIndex{ filePath.find_last_of("/\\") };
		return separatorIndex == std::string_view::npos ? filePath : filePath.substr(separatorIndex + 1);
	}

}

namespace BinaryEngine::Detail {

	void WriteAssertFailure([[maybe_unused]] LogSource source, std::string_view headline, std::string_view condition, std::string_view filePath, int lineNumber, std::string_view message)
	{
		std::string failureLine{ fmt::format("{}: {} ({}:{})", headline, condition, GetFileName(filePath), lineNumber) };
		if (!message.empty())
		{
			failureLine.append(": ").append(message);
		}

#ifndef BINARY_ENGINE_LOGGING_DISABLE
		if (Log::IsInitialised())
		{
			const std::shared_ptr<spdlog::logger>& logger{ source == LogSource::Core ? Log::Get().GetCoreLogger() : Log::Get().GetAppLogger() };
			logger->critical(failureLine);
			logger->flush();
			return;
		}
#endif

		std::fputs(failureLine.c_str(), stderr);
		std::fputc('\n', stderr);
		std::fflush(stderr);
	}

	bool IsDebuggerAttached()
	{
#if defined(_WIN32)
		return IsDebuggerPresent() != 0;
#else
		return false;
#endif
	}

	void EndProcessAfterAssert()
	{
#if defined(_MSC_VER)
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
		std::abort();
	}

}
