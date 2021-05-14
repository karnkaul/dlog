#pragma once
#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <fmt/format.h>

namespace dl {
#if defined(DLOG_DEBUG)
inline constexpr bool dlog_debug = true;
#else
inline constexpr bool dlog_debug = false;
#endif

///
/// \brief Level of a log message
///
enum class level { debug, info, warning, error, count_ };

///
/// \brief Obtain meta-formatted string
///
std::string format(level level, std::string_view text);
///
/// \brief Print to mapped output (stdout / stderr by default)
///
template <typename... Args>
void log(level level, std::string_view fmt, Args&&... args);

namespace config {
using namespace std::string_view_literals;

inline constexpr std::array g_log_levels = {"Debug"sv, "Info"sv, "Warning"sv, "Error"sv};

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
	struct token;

	///
	/// \brief Register a new function; discard token to unregister
	///
	[[nodiscard]] token add(func f);
	///
	/// \brief Invoke all registered functions
	///
	void operator()(Args... args) const;

  private:
	void remove(std::uint64_t id) noexcept;

	using entry = std::pair<std::uint64_t, func>;
	std::vector<entry> m_entries;
	std::uint64_t m_next = 0;

	friend struct token;
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
} // namespace config
} // namespace dl

// impl

template <typename... Args>
void dl::log(level level, std::string_view fmt, Args&&... args) {
	if (level >= config::g_min_level) {
		extern void log_impl(dl::level level, std::string_view text);
		log_impl(level, fmt::format(fmt, std::forward<Args>(args)...));
	}
}

namespace dl::config {
template <typename... Args>
struct func_list<Args...>::token {
	constexpr token() = default;
	constexpr token(func_list& flist, std::uint64_t id) noexcept;
	constexpr token(token&&) noexcept;
	constexpr token& operator=(token&&) noexcept;
	~token();

  private:
	func_list* p_flist = nullptr;
	std::uint64_t id = 0;
};

template <typename... Args>
constexpr func_list<Args...>::token::token(func_list& flist, std::uint64_t id) noexcept : p_flist(&flist), id(id) {}

template <typename... Args>
constexpr func_list<Args...>::token::token(token&& rhs) noexcept : p_flist(rhs.p_flist), id(rhs.id) {
	rhs.p_flist = nullptr;
	rhs.id = 0;
}

template <typename... Args>
constexpr typename func_list<Args...>::token& func_list<Args...>::token::operator=(token&& rhs) noexcept {
	if (&rhs != this) {
		if (id > 0 && p_flist) { p_flist->remove(id); }
		id = rhs.id;
		p_flist = rhs.p_flist;
		rhs.p_flist = nullptr;
		rhs.id = 0;
	}
	return *this;
}

template <typename... Args>
func_list<Args...>::token::~token() {
	if (id > 0 && p_flist) { p_flist->remove(id); }
}

template <typename... Args>
typename func_list<Args...>::token func_list<Args...>::add(func f) {
	if (f) {
		auto const id = ++m_next;
		m_entries.push_back({id, f});
		return token(*this, id);
	}
	return {};
}

template <typename... Args>
void func_list<Args...>::remove(std::uint64_t id) noexcept {
	auto search = std::remove_if(m_entries.begin(), m_entries.end(), [id](auto& e) -> bool { return e.first == id; });
	m_entries.erase(search, m_entries.end());
}

template <typename... Args>
void func_list<Args...>::operator()(Args... args) const {
	for (auto [_, f] : m_entries) { f(args...); }
}
} // namespace dl::config
