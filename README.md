# Dumb Log

[![Build Status](https://github.com/karnkaul/dlog/actions/workflows/ci.yml/badge.svg)](https://github.com/karnkaul/dlog/actions/workflows/ci.yml)

This is a "dumb simple" logging library that uses [fmt](https://github.com/fmtlib/fmt).

## Features

- Publicly integrated `fmt`
- Reentrant and thread-safe functions
- MSVC Output Window (`OutputDebugStringA`)
- Separate `format()` function
- Log levels: `error`, `warn`, `info`, `debug`
- Meta-format: add `level`, `thread`, and `timestamp` text around message
- Async file logging (optional)
- Attach custom output `pipe`s
- Customizable output channels (`unsigned char` bit flags)

## Limitations

- `char` and `std::string` support only (no wide strings)
- No support for domain based logging / filtering

## Usage

### Requirements

- CMake 3.14+
- C++17 compiler (and stdlib)

### Steps

1. Clone repo to appropriate subdirectory, say `dumb_log`
1. Add library to project via: `add_subdirectory(dumb_log)` and `target_link_libraries(foo dlog::dlog)`
1. Use via: `#include <dumb_log/dumb_log.hpp>`
1. Use only fmt via: `#include <fmt/format.h>`

### Examples

#### Simple

```cpp
dumb_log::set_log_format("[{level}] [T{thread}] {message} [{timestamp}]");
dumb_log::info("{} and {} successfully initialised", "Instance", "Device");

/* output:
[I] [T0] Instance and Device successfully initialised [21:23:18]
*/
```

#### File Logging

```cpp
{
  // start thread capturing logs asynchronously into passed file path
  // backs up existing file if any (suffix can be customized)
  // obtain RAII handle
  auto handle = dumb_log::file_logger::attach("log.txt");
  // ...
} // thread joined
```

#### Custom Pipes

```cpp
struct my_pipe : dumb_log::pipe {
  dumb_log::level min_level;

  my_pipe(dumb_log::level min_level) noexcept : min_level(min_level) {}

  void operator()(dumb_log::level l, std::string_view line) const override {
    if (l >= min_level) {
      /* custom logic */ 
    }
  }
};

{
  auto handle = dumb_log::pipe::attach<my_pipe>(dumb_log::level::info);
  // ...
}
```

### Contributing

Pull/merge requests are welcome.
