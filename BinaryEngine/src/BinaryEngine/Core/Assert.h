#pragma once

#include <format>
#include <string_view>
#include <utility>

#include "BinaryEngine/Log/LogSource.h"

namespace BinaryEngine::Detail {

	void WriteAssertFailure(LogSource source, std::string_view headline, std::string_view condition, std::string_view filePath, int lineNumber, std::string_view message);
	bool IsDebuggerAttached();
	[[noreturn]] void EndProcessAfterAssert();

	template<typename... Args>
	void ReportAssertFailure(LogSource source, std::string_view condition, std::string_view filePath, int lineNumber, std::format_string<Args...> format, Args&&... arguments)
	{
		WriteAssertFailure(source, "[Assert] Assertion failed", condition, filePath, lineNumber, std::format(format, std::forward<Args>(arguments)...));
	}

}

#if defined(_MSC_VER)
#define BINARY_ENGINE_ASSERT_STOP()         do { if (::BinaryEngine::Detail::IsDebuggerAttached()) { __debugbreak(); } else { ::BinaryEngine::Detail::EndProcessAfterAssert(); } } while (0)
#else
#define BINARY_ENGINE_ASSERT_STOP()         ::BinaryEngine::Detail::EndProcessAfterAssert()
#endif

#if defined(BINARY_ENGINE_DEBUG_ASSERTS) && !defined(BINARY_ENGINE_ASSERTS_DISABLE)

#define CORE_ASSERT(condition, ...)         do { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::Core, #condition, __FILE__, __LINE__, __VA_ARGS__); BINARY_ENGINE_ASSERT_STOP(); } } while (0)
#define APP_ASSERT(condition, ...)          do { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::App, #condition, __FILE__, __LINE__, __VA_ARGS__); BINARY_ENGINE_ASSERT_STOP(); } } while (0)

#else

#define CORE_ASSERT(condition, ...)         do { if constexpr (false) { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::Core, #condition, __FILE__, __LINE__, __VA_ARGS__); } } } while (0)
#define APP_ASSERT(condition, ...)          do { if constexpr (false) { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::App, #condition, __FILE__, __LINE__, __VA_ARGS__); } } } while (0)

#endif

#if !defined(BINARY_ENGINE_ASSERTS_DISABLE)

#define CORE_RELEASE_ASSERT(condition, ...) do { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::Core, #condition, __FILE__, __LINE__, __VA_ARGS__); BINARY_ENGINE_ASSERT_STOP(); } } while (0)
#define APP_RELEASE_ASSERT(condition, ...)  do { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::App, #condition, __FILE__, __LINE__, __VA_ARGS__); BINARY_ENGINE_ASSERT_STOP(); } } while (0)

#else

#define CORE_RELEASE_ASSERT(condition, ...) do { if constexpr (false) { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::Core, #condition, __FILE__, __LINE__, __VA_ARGS__); } } } while (0)
#define APP_RELEASE_ASSERT(condition, ...)  do { if constexpr (false) { if (!(condition)) { ::BinaryEngine::Detail::ReportAssertFailure(::BinaryEngine::LogSource::App, #condition, __FILE__, __LINE__, __VA_ARGS__); } } } while (0)

#endif
