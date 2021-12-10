#pragma once

namespace dlog {
enum class level { debug, info, warn, error };

constexpr char const* const level_names[] = {"debug", "info", "warn", "error"};
} // namespace dlog
