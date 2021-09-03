#pragma once
#include <fmt/format.h>
#include <dumb_log/level.hpp>

namespace dl {
///
/// \brief Obtain meta-formatted string
///
std::string format(level level, std::string_view text);
///
/// \brief Print to mapped output (stdout / stderr by default)
///
template <typename... Args>
void log(level level, std::string_view fmt, Args const&... args);
} // namespace dl

// impl

template <typename... Args>
void dl::log(level level, std::string_view fmt, Args const&... args) {
	extern void log_impl(dl::level level, std::string_view text);
	log_impl(level, fmt::format(fmt, args...));
}
