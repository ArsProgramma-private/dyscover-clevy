#pragma once
#include <memory>
#include <string>

// Forward declarations of existing enums (to avoid including heavy headers yet)
// Forward-declare enums defined elsewhere to avoid redefining symbols
enum class KeyEventType;
enum class Key;

struct KeyModifiers { bool shift{false}; bool ctrl{false}; bool alt{false}; bool altGr{false}; };

enum class PermissionState { Unknown, Granted, Denied, Limited };

class IKeyboardHandler {
public:
    virtual bool isCapsLockActive() const = 0;
    virtual std::string translate(Key key, const KeyModifiers& mods) = 0;
    virtual bool sendKey(Key key, KeyEventType type) = 0; // false if unsupported
    virtual void startInterception() = 0; // no-op if unsupported
    virtual void stopInterception() = 0;
    virtual PermissionState permissionState() const = 0;
    virtual ~IKeyboardHandler() = default;
};

std::unique_ptr<IKeyboardHandler> CreateKeyboardHandler();
