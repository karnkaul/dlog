#pragma once
#include <memory>
#include <string_view>
#include <dumb_log/level.hpp>

namespace dlog {
///
/// \brief Abstract base class for custom log sinks/hooks
///
class pipe {
  public:
	struct fpath;
	class handle;

	///
	/// \brief Attach a custom pipe
	///
	template <typename T, typename... Args>
	[[nodiscard]] static handle attach_pipe(Args&&... args);
	static std::size_t attach_count() noexcept;

	virtual ~pipe() = default;

	virtual void operator()(level l, std::string_view line) const = 0;

  protected:
	static handle attach(std::unique_ptr<pipe>&& pipe);
};

struct pipe::fpath {
	char const* path;
	std::string_view backup_suffix;

	constexpr fpath(char const* path, std::string_view backup_suffix = ".bak") noexcept : path(path), backup_suffix(backup_suffix) {}
};

class pipe::handle {
  public:
	constexpr handle() = default;
	handle(handle&& rhs) noexcept : handle() { std::swap(m_id, rhs.m_id); }
	handle& operator=(handle rhs) noexcept { return (std::swap(m_id, rhs.m_id), *this); }
	~handle();

	explicit constexpr operator bool() const noexcept { return m_id >= 0; }

  private:
	explicit constexpr handle(int id) noexcept : m_id(id) {}
	int m_id = -1;
	friend class pipe;
};

// impl

template <typename T, typename... Args>
pipe::handle pipe::attach_pipe(Args&&... args) {
	static_assert(std::is_base_of_v<pipe, T> && std::is_constructible_v<T, Args...>);
	return attach(std::make_unique<T>(std::forward<Args>(args)...));
}
} // namespace dlog
