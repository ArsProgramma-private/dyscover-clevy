// KeyboardHandlerChromeOS.cpp - ChromeOS variant: restricted interception/injection
#include "KeyboardHandler.h"
#include "Keys.h"
#include <string>
#include <cctype>

// ChromeOS build often resembles Linux environment but with tightened sandbox
// constraints. For now we offer the same deterministic translation fallback
// as Linux/mac with no actual interception or injection. Permission probing
// returns Denied for interception/injection while translation still works.

struct KeyMapping { Key key; int code; };
static constexpr KeyMapping s_keyMappings[] = {
    { Key::Backspace, 0x0E }, { Key::Tab, 0x0F }, { Key::Enter, 0x1C },
    { Key::Shift, 0x2A }, { Key::Ctrl, 0x1D }, { Key::Esc, 0x01 },
    { Key::CapsLock, 0x3A }, { Key::Space, 0x39 },
    { Key::A, 0x1E }, { Key::B, 0x30 }, { Key::C, 0x2E }, { Key::D, 0x20 },
    { Key::E, 0x12 }, { Key::F, 0x21 }, { Key::G, 0x22 }, { Key::H, 0x23 },
    { Key::I, 0x17 }, { Key::J, 0x24 }, { Key::K, 0x25 }, { Key::L, 0x26 },
    { Key::M, 0x32 }, { Key::N, 0x31 }, { Key::O, 0x18 }, { Key::P, 0x19 },
    { Key::Q, 0x10 }, { Key::R, 0x13 }, { Key::S, 0x1F }, { Key::T, 0x14 },
    { Key::U, 0x16 }, { Key::V, 0x2F }, { Key::W, 0x11 }, { Key::X, 0x2D },
    { Key::Y, 0x15 }, { Key::Z, 0x2C },
    { Key::One, 0x02 }, { Key::Two, 0x03 }, { Key::Three, 0x04 }, { Key::Four, 0x05 },
    { Key::Five, 0x06 }, { Key::Six, 0x07 }, { Key::Seven, 0x08 }, { Key::Eight, 0x09 },
    { Key::Nine, 0x0A }, { Key::Zero, 0x0B },
};

static int KeyCodeFromKey(Key key){ for(auto m: s_keyMappings) if(m.key==key) return m.code; return -1; }

class KeyboardHandlerChromeOS : public IKeyboardHandler {
public:
    KeyboardHandlerChromeOS() = default;
    ~KeyboardHandlerChromeOS() = default;

    bool isCapsLockActive() const override { return m_capsCached; }

    std::string translate(Key key, const KeyModifiers& mods) override {
        if (key >= Key::A && key <= Key::Z) {
            char base = 'a' + static_cast<int>(key) - static_cast<int>(Key::A);
            if (mods.shift) base = static_cast<char>(::toupper(base));
            return std::string(1, base);
        }
        switch (key) {
            case Key::Zero: return mods.shift ? ")" : "0";
            case Key::One: return mods.shift ? "!" : "1";
            case Key::Two: return mods.shift ? "@" : "2";
            case Key::Three: return mods.shift ? "#" : "3";
            case Key::Four: return mods.shift ? "$" : "4";
            case Key::Five: return mods.shift ? "%" : "5";
            case Key::Six: return mods.shift ? "^" : "6";
            case Key::Seven: return mods.shift ? "&" : "7";
            case Key::Eight: return mods.shift ? "*" : "8";
            case Key::Nine: return mods.shift ? "(" : "9";
            case Key::Minus: return mods.shift ? "_" : "-";
            case Key::Equal: return mods.shift ? "+" : "=";
            case Key::OpenBracket: return mods.shift ? "{" : "[";
            case Key::CloseBracket: return mods.shift ? "}" : "]";
            case Key::Backslash: return mods.shift ? "|" : "\\";
            case Key::Semicolon: return mods.shift ? ":" : ";";
            case Key::Apostrophe: return mods.shift ? "\"" : "'";
            case Key::Comma: return mods.shift ? "<" : ",";
            case Key::Dot: return mods.shift ? ">" : ".";
            case Key::Slash: return mods.shift ? "?" : "/";
            case Key::Backtick: return mods.shift ? "~" : "`";
            case Key::Space: return " ";
            default: break;
        }
        return std::string();
    }

    bool sendKey(Key, KeyEventType) override {
        // Injection intentionally unsupported under ChromeOS constraints.
        return false;
    }

    void startInterception() override { /* Restricted: no interception */ m_permission = PermissionState::Denied; }
    void stopInterception() override { }

    PermissionState permissionState() const override { return m_permission; }

private:
    bool m_capsCached{false};
    PermissionState m_permission{PermissionState::Denied};
};

std::unique_ptr<IKeyboardHandler> CreatePlatformKeyboardHandler() {
    return std::unique_ptr<IKeyboardHandler>(new KeyboardHandlerChromeOS());
}
