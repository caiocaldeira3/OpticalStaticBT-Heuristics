#pragma once

#include <iostream>

enum class LogLevel { Debug = 0, Info = 1, Warn = 2, Error = 3, None = 4 };

inline LogLevel g_logLevel = LogLevel::Info;

struct LogStream {
    bool active;

    template<typename T>
    const LogStream& operator<<(const T& val) const {
        if (active) std::cout << val;
        return *this;
    }

    const LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) const {
        if (active) manip(std::cout);
        return *this;
    }
};

inline LogStream log(LogLevel level) {
    return LogStream{ level >= g_logLevel };
}
