// KeyboardHandlerPermissionTest.cpp - Failing test for T042
#include "platform/KeyboardHandler.h"
#include "Keys.h"
#include <iostream>

int main(){
    auto handler = CreateKeyboardHandler();
    if(!handler){ std::cerr << "No handler"; return 100; }

    // If permission is Granted on macOS we expect sendKey for letter A to be FALSE until injection implemented.
    // Current macOS handler returns true (mapping only) so this keeps test red on mac builds.
    if(handler->permissionState() == PermissionState::Granted) {
        bool injected = handler->sendKey(Key::A, (KeyEventType)0); // cast placeholder enum value
        if(injected) {
            std::cerr << "RED: Expected sendKey to fail under Granted (injection not implemented)" << std::endl;
            return 1;
        }
    } else {
        // For Denied we require sendKey to fail; ChromeOS already meets this.
        bool injected = handler->sendKey(Key::A, (KeyEventType)0);
        if(injected) {
            std::cerr << "RED: Expected sendKey false when permission denied" << std::endl;
            return 2;
        }
    }

    std::cout << "Permission test unexpectedly passed" << std::endl;
    return 0;
}
