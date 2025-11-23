#include "KeyboardHandler.h"

namespace {
    class StubKeyboardHandler : public IKeyboardHandler {
    public:
        bool isCapsLockActive() const override { return true; }
        std::string translate(Key, const KeyModifiers&) override { return std::string("a"); }
        bool sendKey(Key, KeyEventType) override { return true; }
        void startInterception() override { /* no-op */ }
        void stopInterception() override { /* no-op */ }
        PermissionState permissionState() const override { return PermissionState::Granted; }
    };
}

std::unique_ptr<IKeyboardHandler> CreateKeyboardHandler() {
#if defined(__USE_PLATFORM_DETECTOR__)
    // When building the real application, prefer platform-backed handler if
    // available. Platform implementations provide CreatePlatformKeyboardHandler.
    extern std::unique_ptr<IKeyboardHandler> CreatePlatformKeyboardHandler();
    try {
        return CreatePlatformKeyboardHandler();
    } catch (...) {
        return std::unique_ptr<IKeyboardHandler>(new StubKeyboardHandler());
    }
#else
    return std::unique_ptr<IKeyboardHandler>(new StubKeyboardHandler());
#endif
}
