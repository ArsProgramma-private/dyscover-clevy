#pragma once
#include <string>

namespace PlatformUtils {
    bool extractVidPid(const std::string& hardwareId, std::string& vid, std::string& pid);
    std::string platformName();
}
