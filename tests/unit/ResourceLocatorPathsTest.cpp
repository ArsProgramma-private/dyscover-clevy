// ResourceLocatorPathsTest.cpp - Extended failing test for T054
#include "platform/ResourceLocator.h"
#include <iostream>
#include <algorithm>

static bool endsWith(const std::string& s, const std::string& suffix){
    return s.size() >= suffix.size() && s.compare(s.size()-suffix.size(), suffix.size(), suffix)==0;
}

int main() {
    auto locator = CreateResourceLocator();
    // Expect platform-specific icon extension logic (not yet implemented): caller should pass base name without extension.
    auto icon = locator->iconFile("Icon");
#if defined(_WIN32)
    if(!endsWith(icon, "Icon.ico")) { std::cerr << "RED: Expected Windows icon to end with Icon.ico got " << icon << std::endl; return 1; }
#elif defined(__APPLE__)
    if(!endsWith(icon, "Icon.icns")) { std::cerr << "RED: Expected macOS icon to end with Icon.icns got " << icon << std::endl; return 1; }
#else
    // Linux/ChromeOS should use .ico fallback
    if(!endsWith(icon, "Icon.ico")) { std::cerr << "RED: Expected Linux/ChromeOS icon to end with Icon.ico got " << icon << std::endl; return 1; }
#endif

    auto splash = locator->splashBitmap();
    // Expect bitmap file naming with exact case 'Splash.bmp'
    if(!endsWith(splash, "Splash.bmp")) { std::cerr << "RED: Expected splash path to end with Splash.bmp got " << splash << std::endl; return 2; }

    auto audio = locator->audioDir();
    // Expect '/audio' directory specifically (trailing slash optional)
    if(audio.find("/audio") == std::string::npos && audio.find("\\audio") == std::string::npos) { std::cerr << "RED: Expected audio dir path to include /audio got " << audio << std::endl; return 3; }

    auto tts = locator->ttsDir();
    // Expect nested data directory: /tts/data/ (not implemented yet)
    if(tts.find("/tts/data") == std::string::npos && tts.find("\\tts\\data") == std::string::npos) { std::cerr << "RED: Expected tts dir path to contain /tts/data got " << tts << std::endl; return 4; }

    auto lang = locator->translationsDir();
    // Expect language directory naming using the actual language code (nl or nl_be) not generic 'lang'
    if(lang.find("/lang/nl") == std::string::npos && lang.find("/lang/nl_be") == std::string::npos) {
        std::cerr << "RED: Expected translations path under /lang/<code> got " << lang << std::endl; return 5; }

    std::cout << "ResourceLocatorPathsTest unexpectedly passed (locator fully implemented)" << std::endl;
    return 0;
}
