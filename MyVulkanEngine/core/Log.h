#pragma once

#include <assert.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace MVE
{
class Log
{
  public:
	static void Init()
	{
		console_logger->set_pattern("[%H:%M:%S] [%^%l%$] [thread %t] %v");
		error_logger->set_pattern("%^>>>>>> ERROR <<<<<<\nAt: %@\n[%H:%M:%S] [thread %t] %v%$");
		spdlog::set_default_logger(error_logger);

#ifndef NDEBUG
		spdlog::set_level(spdlog::level::trace);
#else
		spdlog::set_level(spdlog::level::warn);
#endif
	}

	static auto& GetLogger() { return console_logger; }
	static auto& GetErrorLogger() { return error_logger; }

  private:
	inline static auto console_logger = spdlog::stdout_color_mt("console");
	inline static auto error_logger	  = spdlog::stderr_color_mt("stderr");
};

// clang-format off
#define MVE_TRACE(...)      Log::GetLogger()->trace(__VA_ARGS__)
#define MVE_DEBUG(...)      Log::GetLogger()->debug(__VA_ARGS__)
#define MVE_INFO(...)       Log::GetLogger()->info(__VA_ARGS__)
#define MVE_WARN(...)       Log::GetLogger()->warn(__VA_ARGS__)
#define MVE_ERROR(...)      SPDLOG_ERROR(__VA_ARGS__)
#define MVE_CRITICAL(...)   SPDLOG_CRITICAL(__VA_ARGS__)

#define MVE_BREAK() assert(false)
#define MVE_ASSERT(condition, ...) if(!(condition)) {MVE_ERROR(__VA_ARGS__); MVE_BREAK();}
#define MVE_COMPILE_ASSERT(condition, msg) static_assert(condition, msg)
// clang-format on

} // namespace MVE