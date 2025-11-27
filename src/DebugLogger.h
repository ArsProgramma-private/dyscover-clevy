// Simple file logger for debug diagnostics
#pragma once

#include <string>

namespace DebugLogger {
    // Append `message` to a per-executable log file (new line appended).
    void Append(const std::string& message);
}
