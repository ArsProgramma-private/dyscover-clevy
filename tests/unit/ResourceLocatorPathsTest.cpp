// ResourceLocatorPathsTest.cpp - Failing skeleton for T020
#include "platform/ResourceLocator.h"
#include <iostream>

int main() {
    auto locator = CreateResourceLocator();
    auto icon = locator->iconFile("Icon.ico");
    if(icon.find("Icon.ico") == std::string::npos) {
        std::cerr << "Expected icon path to include Icon.ico (will refine per-platform)" << std::endl;
        return 1;
    }
    auto splash = locator->splashBitmap();
    if(splash.empty()) {
        std::cerr << "Expected splash bitmap path non-empty" << std::endl;
        return 2;
    }
    auto audio = locator->audioDir();
    if(audio.find("audio") == std::string::npos) {
        std::cerr << "Expected audio dir path to contain 'audio'" << std::endl;
        return 3;
    }
    auto tts = locator->ttsDir();
    if(tts.find("tts") == std::string::npos) {
        std::cerr << "Expected tts dir path to contain 'tts'" << std::endl;
        return 4;
    }
    auto lang = locator->translationsDir();
    if(lang.find("lang") == std::string::npos) {
        std::cerr << "Expected translations dir path to contain 'lang'" << std::endl;
        return 5;
    }
    std::cout << "ResourceLocatorPathsTest skeleton executed (expected refinement)" << std::endl;
    return 0;
}
