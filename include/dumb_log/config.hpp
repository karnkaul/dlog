#pragma once
#include <algorithm>
#include <array>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <dumb_log/level.hpp>

namespace dl::config {
using namespace std::string_view_literals;

#if defined(DLOG_DEBUG)
inline constexpr bool dlog_debug = true;
#else
inline constexpr bool dlog_debug = false;
#endif

inline constexpr std::array g_log_levels = {"Debug"sv, "Info"sv, "Warn"sv, "Error"sv};

///
/// \brief Where to send output to
///
enum class output { std_out, std_err, none, count_ };
///
/// \brief Mapping of level to output (unmapped == output::std_out)
///
inline std::unordered_map<level, output> g_output_map = {{level::error, output::std_err}};

///
/// \brief List of function pointer callbacks
///
template <typename... Args>
class func_list final {
  public:
	///
	/// \brief Callback signature
	///
	using func = void (*)(Args...);
	///
	/// \brief Token representing registration (registering func_list object must outlive all tokens)
	///
	struct handle;

	///
	/// \brief Register a new function; discard token to unregister
	///
	[[nodiscard]] handle add(func f);
	///
	/// \brief Invoke all registered functions
	///
	template <typename... T>
	void operator()(T&&... args) const;

  private:
	void remove(std::uint64_t id) noexcept;

	using entry = std::pair<std::uint64_t, func>;
	std::vector<entry> m_entries;
	std::uint64_t m_next = 0;

	friend struct handle;
};

using on_log = func_list<std::string_view, level>;
///
/// \brief Minimum log level
///
inline level g_min_level = dlog_debug ? level::debug : level::info;
///
/// \brief Format to log each message as; available metas: level, thread, message, timestamp
///
inline std::string_view g_meta_format = "[{level}] [T{thread}] {message} [{timestamp}]";
///
/// \brief Registerable callback list for each log message
///
inline on_log g_on_log;
///
/// \brief ID for this thread used in logging
///
std::uint32_t log_thread_id();

// impl

template <typename... Args>
struct func_list<Args...>::handle {
	constexpr handle() = default;
	constexpr handle(func_list& flist, std::uint64_t id) noexcept : flist(&flist), id(id) {}
	constexpr handle(handle&& rhs) noexcept { exchg(*this, rhs); }
	constexpr handle& operator=(handle rhs) noexcept { return (exchg(*this, rhs), *this); }
	~handle() noexcept;

  private:
	constexpr void exchg(handle& lhs, handle& rhs) noexcept;

	func_list* flist = nullptr;
	std::uint64_t id = 0;
};

template <typename... Args>
constexpr void func_list<Args...>::handle::exchg(handle& lhs, handle& rhs) noexcept {
	std::swap(lhs.id, rhs.id);
	std::swap(lhs.flist, rhs.flist);
}

template <typename... Args>
func_list<Args...>::handle::~handle() noexcept {
	if (id > 0 && flist) { flist->remove(id); }
}

template <typename... Args>
typename func_list<Args...>::handle func_list<Args...>::add(func f) {
	if (f) {
		auto const id = ++m_next;
		m_entries.push_back({id, f});
		return handle(*this, id);
	}
	return {};
}

template <typename... Args>
void func_list<Args...>::remove(std::uint64_t id) noexcept {
	auto search = std::remove_if(m_entries.begin(), m_entries.end(), [id](auto& e) -> bool { return e.first == id; });
	m_entries.erase(search, m_entries.end());
}

template <typename... Args>
template <typename... T>
void func_list<Args...>::operator()(T&&... args) const {
	for (auto [_, f] : m_entries) { f(std::forward<T>(args)...); }
}
} // namespace dl::config
