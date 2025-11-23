// KeyboardHandlerCapsLockTest.cpp - Failing test for T041
#include "platform/KeyboardHandler.h"
#include <iostream>

int main(){
    auto handler = CreateKeyboardHandler();
    if(!handler){ std::cerr << "No handler"; return 100; }
    // Expect caps lock active (will fail until real toggle/state implemented on non-Windows platforms).
    if(!handler->isCapsLockActive()) {
        std::cerr << "RED: Expected isCapsLockActive() true" << std::endl;
        return 1;
    }
    std::cout << "CapsLock test unexpectedly passed" << std::endl;
    return 0;
}
