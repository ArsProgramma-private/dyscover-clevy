#ifndef SUPPORTED_DEVICES_H
#define SUPPORTED_DEVICES_H

// Embedded list of supported USB devices.
// This list is compiled into the binary to prevent runtime modification.
// Modify entries and rebuild to change supported devices.
// Accessibility note: Device descriptions are for logging/diagnostics; ensure any
// UI exposing device info uses readable, high-contrast text and conveys state without relying solely on color.

struct SupportedDevice {
    const char* vendorId;    // 4 uppercase hex characters (e.g., "04B4")
    const char* productId;   // 4 uppercase hex characters (e.g., "0101")
    const char* description; // Optional description for diagnostics
};

inline constexpr SupportedDevice SUPPORTED_DEVICES[] = {
    {"04B4", "0101", "Cypress Semiconductor Device"}
    // Additional devices can be added here.
};

#include <string>
#include <algorithm>

inline std::string NormalizeHex4(const std::string& in)
{
    std::string s = in;
    // Trim whitespace
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    // Uppercase
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(::toupper(c)); });
    // Pad or truncate to 4
    if (s.size() < 4) s.append(4 - s.size(), '0');
    if (s.size() > 4) s = s.substr(0,4);
    return s;
}

inline bool IsSupported(const std::string& vid, const std::string& pid)
{
    std::string v = NormalizeHex4(vid);
    std::string p = NormalizeHex4(pid);
    for (const auto& dev : SUPPORTED_DEVICES) {
        if (v == dev.vendorId && p == dev.productId) {
            return true;
        }
    }
    return false;
}

// Future optimization: if SUPPORTED_DEVICES grows beyond ~200 entries,
// switch to an unordered_set<uint32_t> combining (VID<<16)|PID to achieve O(1) lookups.

#endif // SUPPORTED_DEVICES_H
