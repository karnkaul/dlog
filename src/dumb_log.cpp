#include <atomic>
#include <cstdio>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <fmt/chrono.h>
#include <dumb_log/dumb_log.hpp>
#include <dumb_log/file_logger.hpp>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace {
std::unordered_map<int, std::unique_ptr<dlog::pipe>> g_pipes;
int g_next_pipe_id = 0;
std::string_view g_log_format = dlog::default_log_format;
dlog::level g_min_level;
auto g_channels = dlog::channel_flags{0xff};

#if defined(_WIN32)
std::atomic<bool> g_win_utf8_init = false;
void win_utf8_init() {
	if (!g_win_utf8_init) {
		SetConsoleOutputCP(CP_UTF8);
		g_win_utf8_init = true;
	}
}

#if defined(DLOG_OUTPUT_DEBUG_STRING)
std::wstring utf8_to_utf16(std::string_view text) {
	static constexpr DWORD flags = MB_ERR_INVALID_CHARS;
	auto const u8_size = text.size();
	if (u8_size > static_cast<std::size_t>(std::numeric_limits<int>::max())) { return L"[utf-8 string too large]"; }
	auto const iu8_size = static_cast<int>(u8_size);
	int const iu16_size = ::MultiByteToWideChar(CP_UTF8, flags, text.data(), iu8_size, nullptr, 0);
	if (iu16_size == 0) { return L"[MultiByteToWideChar failed]"; }
	auto const u16_size = static_cast<std::size_t>(iu16_size);
	std::wstring ret;
	ret.resize(u16_size + 1U);
	int const res = ::MultiByteToWideChar(CP_UTF8, flags, text.data(), iu8_size, &ret[0], iu16_size);
	if (res == 0) { return L"[MultiByteToWideChar failed]"; }
	ret[u16_size] = L'\n';
	return ret;
}
#endif
#endif
} // namespace

// pipe

dlog::pipe::handle::~handle() {
	if (*this) { g_pipes.erase(m_id); }
}

dlog::pipe::handle dlog::pipe::attach(std::unique_ptr<pipe>&& pipe) {
	if (pipe) {
		handle ret(g_next_pipe_id++);
		g_pipes[ret.m_id] = std::move(pipe);
		return ret;
	}
	return {};
}

std::size_t dlog::pipe::attach_count() noexcept { return g_pipes.size(); }

std::string dlog::format(level lvl, std::string_view text) {
	static constexpr char letters[] = {'D', 'I', 'W', 'E'};
	using namespace fmt::literals;
	using clock = std::chrono::system_clock;
	auto const sub = [](std::string_view str) { return g_log_format.find(str) != std::string::npos; };
	char const pre = letters[std::size_t(lvl)];
	std::string const ts = sub("timestamp") ? fmt::format("{:%H:%M:%S}", fmt::localtime(clock::to_time_t(clock::now()))) : std::string();
	int const id = sub("thread") ? this_thread_id() : 0;
	return fmt::format(g_log_format, "level"_a = pre, "thread"_a = id, "message"_a = text, "timestamp"_a = ts);
}

void dlog::log(level lvl, std::string_view text, channel ch) {
#if defined(_WIN32)
	win_utf8_init();
#endif
	if (lvl >= g_min_level && (ch == 0x0 || (ch & g_channels.bits) != 0x0)) {
		auto const line = format(lvl, text);
		for (auto const& [_, pipe] : g_pipes) {
			if (pipe) { (*pipe)(lvl, line); }
		}
		std::fprintf(lvl == level::error ? stderr : stdout, "%s\n", line.data());
#if defined(_WIN32) && defined(DLOG_OUTPUT_DEBUG_STRING)
		if (IsDebuggerPresent()) { OutputDebugStringW(utf8_to_utf16(text).data()); }
#endif
	}
}

int dlog::this_thread_id() {
	static int next_id = 0;
	static std::unordered_map<std::thread::id, int> thread_ids;
	static std::mutex mutex;
	auto lock = std::scoped_lock(mutex);
	auto const id = std::this_thread::get_id();
	if (auto search = thread_ids.find(id); search != thread_ids.end()) { return search->second; }
	auto [ret, _] = thread_ids.emplace(id, next_id++);
	return ret->second;
}

dlog::level dlog::min_level() noexcept { return g_min_level; }
void dlog::set_min_level(level s) noexcept { g_min_level = s; }
std::string_view dlog::log_format() noexcept { return g_log_format; }
void dlog::set_log_format(std::string_view format) noexcept { g_log_format = format; }
dlog::channel_flags dlog::channels() noexcept { return g_channels; }
void dlog::set_channels(channel_flags set, channel_flags unset) noexcept {
	g_channels.bits &= ~unset.bits;
	g_channels.bits |= set.bits;
}
