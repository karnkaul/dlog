## Dumb Log

[![Build status](https://ci.appveyor.com/api/projects/status/v1b9ri4dtgjtpn2u?svg=true)](https://ci.appveyor.com/project/karnkaul/dlog)

This is a "dumb simple" logging library that uses [fmt](https://github.com/fmtlib/fmt).

### Features

- Publicly integrated `fmt`
- Reentrant and thread-safe
- Separate `format()` function independent of printing (`log()`)
- Log levels: `error`, `warning`, `info`, `debug`
- Outputs mapped to levels: `std_out`, `std_err`, `none`
- Meta-format: add `level`, `thread`, and `timestamp` text around message
- On Log callback: register/unregister per-message function pointer callbacks

### Usage

**Requirements**

- Windows / Linux (any architecture)
- CMake
- C++17 compiler (and stdlib)

**Steps**

1. Clone repo to appropriate subdirectory, say `dumb_log`
1. Add library to project via: `add_subdirectory(dumb_log)` and `target_link_libraries(foo dlog)`
1. Use via: `#include <dumb_log/log.hpp>`
1. Use only fmt via: `#include <fmt/format.h>`

**Example**

```cpp
#include <dumb_log/log.hpp>

int main() {
  dl::g_meta_format = "[{level}] [T{thread}] {message} [{timestamp}]";
  dl::log(dl::level::info, "{} and {} successfully initialised", "Instance", "Device");
}

/* output:
[I] [T0] Instance and Device successfully initialised [21:23:18]
*/
```

### Contributing

Pull/merge requests are welcome.
