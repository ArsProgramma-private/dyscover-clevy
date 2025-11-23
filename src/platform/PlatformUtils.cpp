#include "PlatformUtils.h"
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#elif defined(__linux__)
#include <cstring>
#endif

namespace PlatformUtils {
    bool extractVidPid(const std::string& hardwareId, std::string& vid, std::string& pid) {
        // Simple placeholder: look for VID_XXXX and PID_XXXX
        auto vpos = hardwareId.find("VID_");
        if (vpos == std::string::npos) return false;
        auto ppos = hardwareId.find("PID_", vpos + 4);
        if (ppos == std::string::npos) return false;
        if (vpos + 8 > hardwareId.size() || ppos + 8 > hardwareId.size()) return false;
        vid = hardwareId.substr(vpos + 4, 4);
        pid = hardwareId.substr(ppos + 4, 4);
        return true;
    }
    std::string platformName() {
    #if defined(_WIN32)
        return "windows";
    #elif defined(__APPLE__)
        return "mac";
    #elif defined(__linux__)
        return "linux";
    #else
        return "unknown";
    #endif
    }
}
