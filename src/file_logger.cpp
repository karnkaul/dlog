#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>
#include <thread>
#include <vector>
#include <dumb_log/dumb_log.hpp>
#include <dumb_log/file_logger.hpp>

namespace dlog {
namespace {
bool make_backup(char const* path, std::string_view suffix, std::string& out_path) {
	if (std::filesystem::exists(path)) {
		out_path = path + std::string(suffix);
		std::filesystem::rename(path, out_path);
		if (!std::filesystem::exists(out_path) || std::filesystem::exists(path)) { return false; }
	}
	return true;
}
} // namespace

struct file_logger::impl {
	std::string path;
	std::string backup_path;
	std::vector<std::string> queue;
	std::condition_variable cv;
	std::mutex mutex;
	std::thread thread;
	std::atomic<bool> stop;
	level min_level = level::debug;
};

dlog::file_logger::handle dlog::file_logger::attach(pipe::fpath const& path, std::optional<level> custom) {
	if (!custom) { custom = min_level(); }
	std::string backup_path;
	if (!make_backup(path.path, path.backup_suffix, backup_path)) { return {}; }
	if (auto clear_file = std::ofstream(path.path); !clear_file) { return {}; }
	auto impl = std::make_unique<file_logger::impl>();
	impl->backup_path = std::move(backup_path);
	impl->path = path.path;
	impl->min_level = *custom;
	return pipe::attach(std::unique_ptr<file_logger>(new file_logger(std::move(impl))));
}

file_logger::file_logger(std::unique_ptr<impl>&& impl) noexcept : m_impl(std::move(impl)) {
	m_impl->stop.store(false);
	m_impl->queue.reserve(16);
	m_impl->thread = std::thread([impl = m_impl.get()]() {
		while (!impl->stop.load()) {
			auto lock = std::unique_lock(impl->mutex);
			impl->cv.wait(lock, [impl]() { return impl->stop.load() || !impl->queue.empty(); });
			if (auto file = std::ofstream(impl->path, std::ios::app)) {
				for (auto const& line : impl->queue) { file << line << '\n'; }
				impl->queue.clear();
			}
		}
	});
}

file_logger::~file_logger() {
	m_impl->stop.store(true);
	m_impl->cv.notify_one();
	m_impl->thread.join();
}

void file_logger::operator()(level l, std::string_view const line) const {
	if (l >= m_impl->min_level) {
		auto lock = std::unique_lock(m_impl->mutex);
		m_impl->queue.push_back(std::string(line));
		lock.unlock();
		m_impl->cv.notify_one();
	}
}

std::string_view file_logger::path() const noexcept { return m_impl->path; }
std::string_view file_logger::backed_up_path() const noexcept { return m_impl->backup_path; }
} // namespace dlog
