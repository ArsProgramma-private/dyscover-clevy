// KeyboardHandlerMacTest.cpp - Tests for macOS keyboard handler implementation
#include "platform/KeyboardHandlerMac.cpp" // include implementation directly for test
#include <cassert>
#include <iostream>

int main() {
    auto handler = CreatePlatformKeyboardHandler();
    // permission should be granted
    if (handler->permissionState() != PermissionState::Granted) {
        std::cerr << "Expected permissionState Granted" << std::endl;
        return 1;
    }

    // Caps lock default cached state is false
    if (handler->isCapsLockActive()) {
        std::cerr << "Expected caps lock to be false in default handler" << std::endl;
        return 2;
    }

    // Translate 'a' with no modifiers
    std::string t = handler->translate(Key::A, KeyModifiers{});
    if (t != "a") {
        std::cerr << "Expected translate(Key::A) == 'a', got: '" << t << "'" << std::endl;
        return 3;
    }

    // Translate 'A' with shift
    KeyModifiers mods; mods.shift = true;
    t = handler->translate(Key::A, mods);
    if (t != "A") {
        std::cerr << "Expected translate(Key::A, shift) == 'A', got: '" << t << "'" << std::endl;
        return 4;
    }

    // Translate '1' (number) with no modifiers
    t = handler->translate(Key::One, KeyModifiers{});
    if (t != "1") {
        std::cerr << "Expected translate(Key::One) == '1', got: '" << t << "'" << std::endl;
        return 5;
    }

    // Translate shifted '1' -> '!'
    KeyModifiers modsBang; modsBang.shift = true;
    t = handler->translate(Key::One, modsBang);
    if (t != "!") {
        std::cerr << "Expected '!'/shift fallback for Key::One, got '" << t << "'" << std::endl;
        return 6;
    }

    // Translate punctuation: '-' and shifted '_'
    t = handler->translate(Key::Minus, KeyModifiers{});
    if (t != "-") {
        std::cerr << "Expected '-' for Key::Minus, got '" << t << "'" << std::endl;
        return 7;
    }
    KeyModifiers modsUnderscore; modsUnderscore.shift = true;
    t = handler->translate(Key::Minus, modsUnderscore);
    if (t != "_") {
        std::cerr << "Expected '_' for shifted Key::Minus, got '" << t << "'" << std::endl;
        return 8;
    }

    std::cout << "KeyboardHandlerMacTest passed" << std::endl;
    return 0;
}
