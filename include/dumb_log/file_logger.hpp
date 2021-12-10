#pragma once
#include <optional>
#include <string>
#include <dumb_log/pipe.hpp>

namespace dlog {
///
/// \brief Async file logger
///
class file_logger : public pipe {
  public:
	[[nodiscard]] static handle attach(pipe::fpath const& path, std::optional<level> custom = std::nullopt);

	~file_logger();

	void operator()(level l, std::string_view line) const override;

	///
	/// \brief Path to log file
	///
	std::string_view path() const noexcept;
	///
	/// \brief Path to backed up log file, if any
	///
	std::string_view backed_up_path() const noexcept;

  private:
	struct impl;
	file_logger(std::unique_ptr<impl>&& impl) noexcept;
	std::unique_ptr<impl> m_impl;
};
} // namespace dlog
