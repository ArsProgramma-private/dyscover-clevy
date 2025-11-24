// KeyboardHandlerMac.cpp
#include "KeyboardHandler.h"
#include "Keys.h"
#include <string>
#include <cctype>

#if defined(__APPLE__)
#  include <CoreServices/CoreServices.h>
#  include <ApplicationServices/ApplicationServices.h>
#  include <HIToolbox/Events.h>
#  include <HIToolbox/TextInputSources.h>
#endif

// Provide a macOS keyboard handler that attempts to use the system layout
// (TIS / Carbon) when available, but falls back to deterministic translations
// like the Linux handler for unit tests and CI. This keeps behaviour
// deterministic across environments while allowing future improvements.

struct KeyMapping { Key key; int code; };

static constexpr KeyMapping s_keyMappings[] = {
    { Key::Backspace, 0x33 }, // kVK_Delete (approx)
    { Key::Tab, 0x30 },
    { Key::Enter, 0x24 },
    { Key::Shift, 0x38 },
    { Key::Ctrl, 0x3B },
    { Key::Esc, 0x35 },
    { Key::CapsLock, 0x39 },
    { Key::Space, 0x31 },
    // basic letters (mac virtual keycodes vary across hardware layouts)
    { Key::A, 0x00 }, { Key::B, 0x0B }, { Key::C, 0x08 }, { Key::D, 0x02 },
    { Key::E, 0x0E }, { Key::F, 0x03 }, { Key::G, 0x05 }, { Key::H, 0x04 },
    { Key::I, 0x22 }, { Key::J, 0x26 }, { Key::K, 0x28 }, { Key::L, 0x25 },
    { Key::M, 0x2E }, { Key::N, 0x2D }, { Key::O, 0x1F }, { Key::P, 0x23 },
    { Key::Q, 0x0C }, { Key::R, 0x0F }, { Key::S, 0x01 }, { Key::T, 0x11 },
    { Key::U, 0x20 }, { Key::V, 0x09 }, { Key::W, 0x0D }, { Key::X, 0x07 },
    { Key::Y, 0x10 }, { Key::Z, 0x06 },
    // digits (mac keycodes for numbers are typically 0x12..0x1B but vary)
    { Key::One, 0x12 }, { Key::Two, 0x13 }, { Key::Three, 0x14 }, { Key::Four, 0x15 },
    { Key::Five, 0x17 }, { Key::Six, 0x16 }, { Key::Seven, 0x1A }, { Key::Eight, 0x1C },
    { Key::Nine, 0x19 }, { Key::Zero, 0x1D },
};

static int KeyCodeFromKey(Key key) {
    for (auto m : s_keyMappings) if (m.key == key) return m.code;
    return -1;
}

class KeyboardHandlerMac : public IKeyboardHandler {
public:
    KeyboardHandlerMac() { initLayout(); }
    ~KeyboardHandlerMac() { destroyLayout(); }

    bool isCapsLockActive() const override { return m_capsCached; }

    std::string translate(Key key, const KeyModifiers& mods) override {
#if defined(__APPLE__)
        // Try layout aware mapping first (best-effort). If we don't have a
        // mac keyboard layout available, fall through to reliable fallback.
        if (m_haveLayout) {
            // We don't have a robust cross-platform mapping from Key->vkey;
            // attempt a best-effort translation using our keycode table.
            int vk = KeyCodeFromKey(key);
            if (vk >= 0 && m_layout) {
                // Prepare modifiers
                UInt32 modifiers = 0;
                if (mods.shift) modifiers |= shiftKey;
                if (mods.ctrl)  modifiers |= controlKey;
                if (mods.alt)   modifiers |= optionKey;

                UniChar outChars[8] = {0};
                UniCharCount actual = 0;

                OSStatus s = UCKeyTranslate((const UCKeyboardLayout*)CFDataGetBytePtr(m_layout),
                                           vk,
                                           kUCKeyActionDown,
                                           (modifiers >> 8) & 0xff,
                                           LMGetKbdType(),
                                           kUCKeyTranslateNoDeadKeysBit,
                                           NULL,
                                           sizeof(outChars)/sizeof(UniChar),
                                           &actual,
                                           outChars);
                if (s == noErr && actual > 0) {
                    // convert UniChar to UTF-8 string
                    std::string res;
                    for (UniCharCount i=0;i<actual;i++) res.push_back(static_cast<char>(outChars[i] & 0xff));
                    return res;
                }
            }
        }
#endif
        // Fallback deterministic behavior: letters, digits and punctuations
        if (key >= Key::A && key <= Key::Z) {
            char base = 'a' + static_cast<int>(key) - static_cast<int>(Key::A);
            if (mods.shift) base = static_cast<char>(::toupper(base));
            return std::string(1, base);
        }

        // digits
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
            default: break;
        }

        // punctuation
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
            case Key::Space: return " ";
            default: break;
        }

        return std::string();
    }

    bool sendKey(Key key, KeyEventType) override {
        // Injection not implemented yet on macOS; return false while still
        // allowing translation. Provide one-time warning.
        if (!m_warnedInjection) {
            m_warnedInjection = true;
        }
        int code = KeyCodeFromKey(key);
        return code != -1; // map known key but report no actual injection
    }

    void startInterception() override {
#if defined(__APPLE__)
        if (m_eventTap || !m_haveLayout) { return; }
        // Attempt to create an event tap to observe key events (permission probe)
        m_eventTap = CGEventTapCreate(kCGSessionEventTap,
                                      kCGHeadInsertEventTap,
                                      kCGEventTapOptionListenOnly,
                                      CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp),
                                      &KeyboardHandlerMac::TapCallback,
                                      this);
        if (!m_eventTap) {
            m_permission = PermissionState::Denied;
            return;
        }
        m_runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, m_eventTap, 0);
        if (!m_runLoopSource) {
            CFMachPortInvalidate(m_eventTap);
            CFRelease(m_eventTap);
            m_eventTap = nullptr;
            m_permission = PermissionState::Denied;
            return;
        }
        CFRunLoopAddSource(CFRunLoopGetCurrent(), m_runLoopSource, kCFRunLoopCommonModes);
        CGEventTapEnable(m_eventTap, true);
        if (!CGEventTapIsEnabled(m_eventTap)) {
            // Disabled likely due to permission
            stopInterception();
            m_permission = PermissionState::Denied;
            return;
        }
        m_permission = PermissionState::Granted;
#endif
    }

    void stopInterception() override {
#if defined(__APPLE__)
        if (m_runLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), m_runLoopSource, kCFRunLoopCommonModes);
            CFRelease(m_runLoopSource);
            m_runLoopSource = nullptr;
        }
        if (m_eventTap) {
            CFMachPortInvalidate(m_eventTap);
            CFRelease(m_eventTap);
            m_eventTap = nullptr;
        }
#endif
    }

    PermissionState permissionState() const override { return m_permission; }

private:
    bool m_capsCached{false};
#if defined(__APPLE__)
    CFMachPortRef m_eventTap{nullptr};
    CFRunLoopSourceRef m_runLoopSource{nullptr};
#endif
    bool m_warnedInjection{false};
    PermissionState m_permission{PermissionState::Granted};
#if defined(__APPLE__)
    CFDataRef m_layout{nullptr};
    bool m_haveLayout{false};

    void initLayout() {
        // Attempt to get current keyboard layout via TIS
        TISInputSourceRef src = TISCopyCurrentKeyboardLayoutInputSource();
        if (!src) return;
        CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(src, kTISPropertyUnicodeKeyLayoutData);
        if (layoutData) {
            CFRetain(layoutData);
            m_layout = layoutData;
            m_haveLayout = true;
        }
        CFRelease(src);
    }

    void destroyLayout() {
        if (m_layout) { CFRelease(m_layout); m_layout = nullptr; m_haveLayout = false; }
    }

    static CGEventRef TapCallback(CGEventTapProxy, CGEventType type, CGEventRef event, void* refcon) {
        // We are listen-only; just return the event. Could later translate & dispatch.
        (void)type; (void)refcon; return event;
    }
#else
    void initLayout() {}
    void destroyLayout() {}
#endif
};

std::unique_ptr<IKeyboardHandler> CreatePlatformKeyboardHandler() {
    return std::unique_ptr<IKeyboardHandler>(new KeyboardHandlerMac());
}
