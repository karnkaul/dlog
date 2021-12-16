#pragma once
#include <string_view>
#include <fmt/format.h>
#include <dumb_log/channel_flags.hpp>
#include <dumb_log/level.hpp>

namespace dlog {
///
/// \brief Default format of each log message
///
static constexpr std::string_view default_log_format = "[{level}] [T{thread}] {message} [{timestamp}]";

///
/// \brief Obtain formatted string
/// \returns string formatted according to log_format()
///
std::string format(level lvl, std::string_view text);
///
/// \brief Log text
///
void log(level lvl, std::string_view text, channel ch = 0);
///
/// \brief Log text
///
template <typename... Args>
void log(level lvl, std::string_view fmt, Args const&... args);
template <typename... Args>
void log(level lvl, channel ch, std::string_view fmt, Args const&... args);

template <typename... Args>
void error(std::string_view fmt, Args const&... args);
template <typename... Args>
void warn(std::string_view fmt, Args const&... args);
template <typename... Args>
void info(std::string_view fmt, Args const&... args);
template <typename... Args>
void debug(std::string_view fmt, Args const&... args);

template <typename... Args>
void error(channel ch, std::string_view fmt, Args const&... args);
template <typename... Args>
void warn(channel ch, std::string_view fmt, Args const&... args);
template <typename... Args>
void info(channel ch, std::string_view fmt, Args const&... args);
template <typename... Args>
void debug(channel ch, std::string_view fmt, Args const&... args);

///
/// \brief Obtain ID used for this thread
///
int this_thread_id();
level min_level() noexcept;
void set_min_level(level lvl) noexcept;
std::string_view log_format() noexcept;
void set_log_format(std::string_view format) noexcept;
channel_flags channels() noexcept;
void set_channels(channel_flags set, channel_flags unset = {}) noexcept;

// impl

template <typename... Args>
void log(level const lvl, std::string_view const fmt, Args const&... args) {
	log(lvl, 0x0, fmt, args...);
}

template <typename... Args>
void log(level const lvl, channel const ch, std::string_view const fmt, Args const&... args) {
	if (lvl >= min_level() && (ch == 0x0 || (ch & channels().bits) != 0x0)) { log(lvl, fmt::format(fmt::runtime(fmt), args...)); }
}

template <typename... Args>
void error(std::string_view const fmt, Args const&... args) {
	error(0x0, fmt, args...);
}

template <typename... Args>
void warn(std::string_view const fmt, Args const&... args) {
	warn(0x0, fmt, args...);
}

template <typename... Args>
void info(std::string_view const fmt, Args const&... args) {
	info(0x0, fmt, args...);
}

template <typename... Args>
void debug(std::string_view fmt, Args const&... args) {
	debug(0x0, fmt, args...);
}

template <typename... Args>
void error(channel const ch, std::string_view const fmt, Args const&... args) {
	log(level::error, ch, fmt, args...);
}

template <typename... Args>
void warn(channel const ch, std::string_view const fmt, Args const&... args) {
	log(level::warn, ch, fmt, args...);
}

template <typename... Args>
void info(channel const ch, std::string_view const fmt, Args const&... args) {
	log(level::info, ch, fmt, args...);
}

template <typename... Args>
void debug(channel const ch, std::string_view const fmt, Args const&... args) {
	log(level::debug, ch, fmt, args...);
}
} // namespace dlog
