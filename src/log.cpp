#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <fmt/chrono.h>
#include <dumb_log/config.hpp>
#include <dumb_log/log.hpp>

#if defined(_MSC_VER)
#include <Windows.h>
#endif

namespace dl {
using namespace config;

namespace {
std::mutex g_mutex;

std::uint32_t thread_id() {
	static std::uint32_t next_id = 0;
	static std::unordered_map<std::thread::id, std::uint32_t> thread_ids;
	auto lock = std::scoped_lock<std::mutex>(g_mutex);
	auto const id = std::this_thread::get_id();
	if (auto search = thread_ids.find(id); search != thread_ids.end()) { return search->second; }
	auto [ret, b_result] = thread_ids.insert({id, next_id++});
	return b_result ? ret->second : 0U;
}

bool sub(std::string_view str) { return g_meta_format.find(str) < g_meta_format.size(); }

std::FILE* fout(level level) {
	auto lock = std::scoped_lock<std::mutex>(g_mutex);
	if (auto search = g_output_map.find(level); search != g_output_map.end()) {
		return search->second == output::std_err ? stderr : search->second == output::none ? nullptr : stdout;
	}
	return stdout;
}
} // namespace

std::string format(level level, std::string_view text) {
	using namespace fmt::literals;
	using clock = std::chrono::system_clock;
	char const pre = g_log_levels[std::size_t(level)][0];
	std::string const ts = sub("timestamp") ? fmt::format("{:%H:%M:%S}", fmt::localtime(clock::to_time_t(clock::now()))) : std::string();
	std::uint64_t const id = sub("thread") ? thread_id() : 0;
	return fmt::format(g_meta_format, "level"_a = pre, "thread"_a = id, "message"_a = text, "timestamp"_a = ts);
}

void log_impl(level level, std::string_view text) {
	if (level >= config::g_min_level) {
		if (auto file = fout(level)) {
			auto str = format(level, text);
			g_on_log(str, level);
			str += "\n";
			std::fprintf(file, "%s", str.data());
#if defined(_MSC_VER)
			OutputDebugStringA(str.data());
#endif
		}
	}
}
} // namespace dl
