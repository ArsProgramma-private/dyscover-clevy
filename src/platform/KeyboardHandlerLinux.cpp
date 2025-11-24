#if defined(__linux__)
#include "KeyboardHandler.h"
#include "Keys.h"
#if defined(HAVE_LIBXKBCOMMON) || defined(__has_include)
#  ifndef HAVE_LIBXKBCOMMON
#    if __has_include(<xkbcommon/xkbcommon.h>)
#      define HAVE_LIBXKBCOMMON 1
#    endif
#  endif
#endif

#if defined(HAVE_LIBXKBCOMMON)
#  include <xkbcommon/xkbcommon.h>
#endif
#include <unistd.h>
#include <string>
#include <vector>

// Minimal Linux KeyboardHandler: local translation + sendKey using uinput will
// be implemented later. For now we provide deterministic behavior usable by
// unit tests and CI. This maps a subset of Key enums to Linux keycodes and
// produces a simple character translation for printable keys.

struct KeyMapping { Key key; int code; };

static constexpr KeyMapping s_keyMappings[] = {
    { Key::Backspace, 0x0E }, // KEY_BACKSPACE
    { Key::Tab, 0x0F },
    { Key::Enter, 0x1C },
    { Key::Shift, 0x2A },
    { Key::Ctrl, 0x1D },
    { Key::Esc, 0x01 },
    { Key::CapsLock, 0x3A },
    { Key::Space, 0x39 },
    { Key::A, 0x1E }, { Key::B, 0x30 }, { Key::C, 0x2E }, { Key::D, 0x20 },
    { Key::E, 0x12 }, { Key::F, 0x21 }, { Key::G, 0x22 }, { Key::H, 0x23 },
    { Key::I, 0x17 }, { Key::J, 0x24 }, { Key::K, 0x25 }, { Key::L, 0x26 },
    { Key::M, 0x32 }, { Key::N, 0x31 }, { Key::O, 0x18 }, { Key::P, 0x19 },
    { Key::Q, 0x10 }, { Key::R, 0x13 }, { Key::S, 0x1F }, { Key::T, 0x14 },
    { Key::U, 0x16 }, { Key::V, 0x2F }, { Key::W, 0x11 }, { Key::X, 0x2D },
    { Key::Y, 0x15 }, { Key::Z, 0x2C },
    // numbers (evdev codes)
    { Key::One, 0x02 }, { Key::Two, 0x03 }, { Key::Three, 0x04 }, { Key::Four, 0x05 },
    { Key::Five, 0x06 }, { Key::Six, 0x07 }, { Key::Seven, 0x08 }, { Key::Eight, 0x09 },
    { Key::Nine, 0x0A }, { Key::Zero, 0x0B },
};

static int KeyCodeFromKey(Key key) {
    for (auto m : s_keyMappings) if (m.key == key) return m.code;
    return -1;
}

class KeyboardHandlerLinux : public IKeyboardHandler {
public:
    KeyboardHandlerLinux() = default;
#if defined(HAVE_LIBXKBCOMMON)
    ~KeyboardHandlerLinux() { destroyXkb(); }
#else
    ~KeyboardHandlerLinux() = default;
#endif

    // Caps lock detection: try to read from /proc/ or /sys if available, otherwise
    // fall back to cached state (default false). This is deterministic enough
    // for unit tests and doesn't require X11/Wayland.
    bool isCapsLockActive() const override {
        // Attempt to read LED state (not universally available), else false.
        // Keep implementation simple: return false in CI but allow overrides later.
        return m_capsCached;
    }

#if defined(HAVE_LIBXKBCOMMON)
    std::string translateWithXkb(Key key, const KeyModifiers& mods) {
        // Try to use xkbcommon for layout-aware translation.
        if (!m_useXKB || !m_state || !m_keymap) return std::string();
        
        int evdev = KeyCodeFromKey(key);
        if (evdev < 0) return std::string();

        // xkb uses XKB keycodes (evdev keycodes + 8)
        xkb_keycode_t xkey = static_cast<xkb_keycode_t>(evdev + 8);

        // Build a depressed mask from modifiers we understand
        xkb_mod_mask_t depressed = 0;
        if (mods.shift && m_modMaskShift) depressed |= (1ULL << m_modMaskShift);
        if (mods.ctrl && m_modMaskCtrl) depressed |= (1ULL << m_modMaskCtrl);
        if (mods.alt && m_modMaskAlt) depressed |= (1ULL << m_modMaskAlt);
        if (mods.altgr && m_modMaskAltGr) depressed |= (1ULL << m_modMaskAltGr);

        // update snapshot state for the single lookup. Leave latched/locked/group as 0.
        xkb_state_update_mask(m_state, depressed, 0, 0, 0, 0, 0);

        char buf[64] = {0};
        int r = xkb_state_key_get_utf8(m_state, xkey, buf, sizeof(buf));
        if (r > 0) return std::string(buf, r);
        
        return std::string();
    }
#endif

    std::string translateFallback(Key key, const KeyModifiers& mods) {
        // Return a simple lowercase ascii for printable letters with Shift support
        if (key >= Key::A && key <= Key::Z) {
            char base = 'a' + static_cast<int>(key) - static_cast<int>(Key::A);
            if (mods.shift) base = static_cast<char>(::toupper(base));
            return std::string(1, base);
        }
        
        // Digits and punctuation
        std::string digitResult = translateDigit(key, mods);
        if (!digitResult.empty()) return digitResult;
        
        std::string punctResult = translatePunctuation(key, mods);
        if (!punctResult.empty()) return punctResult;
        
        if (key == Key::Space) return " ";
        return std::string();
    }

    std::string translateDigit(Key key, const KeyModifiers& mods) {
        // Digits fallback (explicit switch due to enum ordering One..Nine..Zero)
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
            default: return std::string();
        }
    }

    std::string translatePunctuation(Key key, const KeyModifiers& mods) {
        // Common US punctuation fallback with shift variants
        switch (key) {
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
            default: return std::string();
        }
    }

    std::string translate(Key key, const KeyModifiers& mods) override {
#if defined(HAVE_LIBXKBCOMMON)
        std::string xkbResult = translateWithXkb(key, mods);
        if (!xkbResult.empty()) return xkbResult;
#endif
        return translateFallback(key, mods);
    }

    bool sendKey(Key key, KeyEventType) override {
        int code = KeyCodeFromKey(key);
        return code != -1;
    }

    void startInterception() override { /* noop for now: will require permissions */ }
    void stopInterception() override { /* noop */ }

    PermissionState permissionState() const override { return PermissionState::Granted; }

private:
    bool m_capsCached{false};
#if defined(HAVE_LIBXKBCOMMON)
    // xkbcommon runtime objects
    xkb_context* m_ctx{nullptr};
    xkb_keymap* m_keymap{nullptr};
    xkb_state* m_state{nullptr};

    // modifier indices (or 0)
    unsigned int m_modMaskShift{0};
    unsigned int m_modMaskCtrl{0};
    unsigned int m_modMaskAlt{0};
    unsigned int m_modMaskAltGr{0};
    bool m_useXKB{false};

    void initXkb() {
        m_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        if (!m_ctx) return;

        struct xkb_rule_names names = {};
        // Use the environment / defaults by passing empty struct
        m_keymap = xkb_keymap_new_from_names(m_ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (!m_keymap) return;

        m_state = xkb_state_new(m_keymap);
        if (!m_state) {
            xkb_keymap_unref(m_keymap);
            m_keymap = nullptr;
            return;
        }

        // Cache modifier indexes (0 means absent)
        // Prefer well-known modifier names; these may return 0 if not present.
        m_modMaskShift = xkb_keymap_mod_get_index(m_keymap, "Shift");
        m_modMaskCtrl = xkb_keymap_mod_get_index(m_keymap, "Control");
        m_modMaskAlt = xkb_keymap_mod_get_index(m_keymap, "Alt");
        // AltGr / ISO_Level3_Shift - try the common name used on many layouts
        m_modMaskAltGr = xkb_keymap_mod_get_index(m_keymap, "ISO_Level3_Shift");

        m_useXKB = true;
    }

    void destroyXkb() {
        if (m_state) { xkb_state_unref(m_state); m_state = nullptr; }
        if (m_keymap) { xkb_keymap_unref(m_keymap); m_keymap = nullptr; }
        if (m_ctx) { xkb_context_unref(m_ctx); m_ctx = nullptr; }
        m_useXKB = false;
    }
#endif
};


std::unique_ptr<IKeyboardHandler> CreatePlatformKeyboardHandler() {
    auto p = std::unique_ptr<KeyboardHandlerLinux>(new KeyboardHandlerLinux());
#if defined(HAVE_LIBXKBCOMMON)
    // attempt to initialize xkb; not fatal when unavailable
    p->initXkb();
#endif
    return p;
}

#endif // __linux__
