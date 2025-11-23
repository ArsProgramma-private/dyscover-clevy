// KeyboardHandlerChromeOSTest.cpp - Tests for ChromeOS keyboard handler implementation
#include "platform/KeyboardHandlerChromeOS.cpp" // include implementation directly
#include <iostream>

int main() {
    auto handler = CreatePlatformKeyboardHandler();
    // Permission expected denied for interception/injection
    if (handler->permissionState() != PermissionState::Denied) {
        std::cerr << "Expected permissionState Denied for ChromeOS handler" << std::endl;
        return 1;
    }

    // Translate 'a'
    std::string t = handler->translate(Key::A, KeyModifiers{});
    if (t != "a") {
        std::cerr << "Expected 'a', got '" << t << "'" << std::endl;
        return 2;
    }

    // Shift 'A'
    KeyModifiers mods; mods.shift = true;
    t = handler->translate(Key::A, mods);
    if (t != "A") {
        std::cerr << "Expected 'A', got '" << t << "'" << std::endl;
        return 3;
    }

    // Digit + shifted symbol
    t = handler->translate(Key::One, KeyModifiers{});
    if (t != "1") { std::cerr << "Expected '1', got '" << t << "'" << std::endl; return 4; }
    KeyModifiers modsBang; modsBang.shift = true;
    t = handler->translate(Key::One, modsBang);
    if (t != "!") { std::cerr << "Expected '!' got '" << t << "'" << std::endl; return 5; }

    // sendKey should return false (unsupported injection)
    if (handler->sendKey(Key::A, KeyEventType::KeyDown)) {
        std::cerr << "Expected sendKey to return false (unsupported)" << std::endl;
        return 6;
    }

    std::cout << "KeyboardHandlerChromeOSTest passed" << std::endl;
    return 0;
}
