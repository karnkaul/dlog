#pragma once

namespace dlog {
///
/// \brief Arbitrary 8-bit flags for filtering logs
///
using channel = unsigned char;

struct channel_flags {
	channel bits{};
};

namespace literals {
constexpr channel_flags operator""_cf(unsigned long long ch) noexcept { return channel_flags{static_cast<unsigned char>(ch)}; }
} // namespace literals
} // namespace dlog
