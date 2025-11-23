// KeyboardHandlerTranslateTest.cpp - Failing skeleton for T018
// Intentionally asserts an advanced modified key translation that
// is NOT implemented yet for Windows platform handler refactor.
// This test should FAIL until T043 (Windows handler) provides
// layout-aware mapping for Alt+Shift combinations.
#include "platform/KeyboardHandler.h"
#include "Keys.h"
#include <iostream>

int main() {
    auto handler = CreateKeyboardHandler();

    // Basic sanity: factory must return a non-null handler.
    if(!handler) {
        std::cerr << "Factory returned null handler" << std::endl;
        return 100;
    }

    // We expect permission probing to eventually yield Granted for platforms
    // that support interception (Windows/macOS/Linux). Current stub may return
    // a different state; treat anything but Granted as a failing (red) condition.
    if(handler->permissionState() != PermissionState::Granted) {
        std::cerr << "RED: Expected permissionState Granted (will implement proper probe)" << std::endl;
        return 1; // Fail early (keeps test red pre-implementation)
    }

    // Caps Lock state query: we will implement real platform check later. Force red now.
    if(!handler->isCapsLockActive()) {
        std::cerr << "RED: Expected CapsLock active mock state (will implement real state)" << std::endl;
        return 2;
    }

    // Choose a representative printable key that should translate differently
    // when ALT+SHIFT modifiers are applied under a real layout (e.g., might produce
    // a diacritic or symbol). Current implementation returns simple ASCII and will
    // not satisfy expectation: keeps test failing until Windows handler + layout mapping added.
    Key key = Key::A;
    KeyModifiers mods{};
    mods.alt = true;
    mods.shift = true;
    auto translated = handler->translate(key, mods);

    // Expected future mapped string (placeholder) chosen to guarantee mismatch now.
    const std::string expected = "Å"; // Will be produced by Alt+Shift+A in extended layout mapping later.
    if(translated != expected) {
        std::cerr << "RED: Expected '" << expected << "' got '" << translated << "'" << std::endl;
        return 3; // keep failing until implemented
    }

    std::cout << "KeyboardHandlerTranslateTest unexpectedly passed (layout mapping implemented)" << std::endl;
    return 0; // Green only after implementation
}
