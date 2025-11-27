// Auto-generated layout module
// Feature: 006-language-resource-optimization (Phase 2)
// Migrated from src/Keys.cpp

#include "Keys.h"
#include "layouts/LayoutLoader.h"
#include "layouts/LayoutRegistry.h"

// Type definitions (copied from Keys.cpp)
enum class CapsLock {
    Ignore,
    Active,
    Inactive,
};

struct KeyTranslationEntry {
    Key input;
    bool shift;
    bool ctrl;
    bool alt;
    std::vector<KeyStroke> output;
    std::string sound;
    bool speak_sentence = false;
    CapsLock capsLock = CapsLock::Ignore;
};

namespace {

static const std::vector<KeyTranslationEntry> g_dutchDefault = {
    { Key::Esc, false, false, false, { { Key::Esc, false, false, false } } },
    { Key::CapsLock, false, false, false, { { Key::CapsLock, false, false, false } } },
    { Key::Up, false, false, false, { { Key::Up, false, false, false } } },
    { Key::Down, false, false, false, { { Key::Down, false, false, false } } },
    { Key::Left, false, false, false, { { Key::Left, false, false, false } } },
    { Key::Right, false, false, false, { { Key::Right, false, false, false } } },
    { Key::Backspace, false, false, false, { { Key::Backspace, false, false, false } } },
    { Key::Space, false, false, false, { { Key::Space, false, false, false } } },
    { Key::Enter, false, false, false, { { Key::Enter, false, false, false } } },
    { Key::Tab, false, false, false, { { Key::Tab, false, false, false } } },
    { Key::Dot, false, false, false, { { Key::Dot, false, false, false } }, std::string(), true },
    { Key::Dot, true, false, false, { { Key::Dot, true, false, false } } },
    { Key::Comma, false, false, false, { { Key::Comma, false, false, false } } },
    { Key::Comma, true, false, false, { { Key::Comma, true, false, false } } },
    { Key::One, false, false, false, { { Key::One, false, false, false } }, "1.wav" },
    { Key::Two, false, false, false, { { Key::Two, false, false, false } }, "2.wav" },
    { Key::Three, false, false, false, { { Key::Three, false, false, false } }, "3.wav" },
    { Key::Four, false, false, false, { { Key::Four, false, false, false } }, "4.wav" },
    { Key::Five, false, false, false, { { Key::Five, false, false, false } }, "5.wav" },
    { Key::Six, false, false, false, { { Key::Six, false, false, false } }, "6.wav" },
    { Key::Seven, false, false, false, { { Key::Seven, false, false, false } }, "7.wav" },
    { Key::Eight, false, false, false, { { Key::Eight, false, false, false } }, "8.wav" },
    { Key::Nine, false, false, false, { { Key::Nine, false, false, false } }, "9.wav" },
    { Key::Zero, false, false, false, { { Key::Zero, false, false, false } }, "0.wav" },
    { Key::One, true, false, false, { { Key::One, true, false, false } }, std::string(), true },
    { Key::Two, true, false, false, { { Key::Two, true, false, false } } },
    { Key::Three, true, false, false, { { Key::Three, true, false, false } } },
    { Key::Four, true, false, false, { { Key::Four, true, false, false } } },
    { Key::Five, true, false, false, { { Key::Five, true, false, false } } },
    //{ Key::Six, true, false, false, { { Key::Six, true, false, false } } },
    { Key::Seven, true, false, false, { { Key::Seven, true, false, false } } },
    { Key::Eight, true, false, false, { { Key::Eight, true, false, false } } },
    { Key::Nine, true, false, false, { { Key::Nine, true, false, false } } },
    { Key::Zero, true, false, false, { { Key::Zero, true, false, false } } },
    { Key::OpenBracket, false, false, false, { { Key::OpenBracket, false, false, false } } },
    { Key::OpenBracket, true, false, false, { { Key::OpenBracket, true, false, false } } },
    { Key::CloseBracket, false, false, false, { { Key::CloseBracket, false, false, false } } },
    { Key::CloseBracket, true, false, false, { { Key::CloseBracket, true, false, false } } },
    { Key::Semicolon, false, false, false, { { Key::Semicolon, false, false, false } } },
    { Key::Semicolon, true, false, false, { { Key::Semicolon, true, false, false } } },
    { Key::Apostrophe, false, false, false, { { Key::Apostrophe, false, false, false } } },
    { Key::Apostrophe, true, false, false, { { Key::Apostrophe, true, false, false } } },
    { Key::Backslash, false, false, false, { { Key::Backslash, false, false, false } } },
    { Key::Backslash, true, false, false, { { Key::Backslash, true, false, false } } },
    { Key::Minus, false, false, false, { { Key::Minus, false, false, false } } },
    { Key::Minus, true, false, false, { { Key::Minus, true, false, false } } },
    { Key::Slash, false, false, false, { { Key::Slash, false, false, false } } },
    { Key::Slash, true, false, false, { { Key::Slash, true, false, false } }, std::string(), true },
    { Key::Equal, false, false, false, { { Key::Equal, false, false, false } } },
    { Key::Equal, true, false, false, { { Key::Equal, true, false, false } } },
    { Key::Ins, false, false, false, { { Key::Ins, false, false, false } } },
    { Key::Del, false, false, false, { { Key::Del, false, false, false } } },
    { Key::Home, false, false, false, { { Key::Home, false, false, false } } },
    { Key::End, false, false, false, { { Key::End, false, false, false } } },
    { Key::PageUp, false, false, false, { { Key::PageUp, false, false, false } } },
    { Key::PageDown, false, false, false, { { Key::PageDown, false, false, false } } },
    { Key::A, false, false, false, { { Key::A, false, false, false } }, "a.wav" },
    { Key::A, true, false, false, { { Key::A, true, false, false } }, "a.wav" },
    { Key::A, false, false, true, { { Key::A, false, false, false } }, "aa.wav" },
    { Key::B, false, false, false, { { Key::B, false, false, false } }, "b.wav" },
    { Key::B, true, false, false, { { Key::B, true, false, false } }, "b.wav" },
    { Key::C, false, false, false, { { Key::C, false, false, false } }, "c.wav" },
    { Key::C, true, false, false, { { Key::C, true, false, false } }, "c.wav" },
    { Key::C, false, true, false, { { Key::C } }, "k.wav" },  // Not a mistake: Ctrl+C should give 'c' with sound 'k'
    { Key::D, false, false, false, { { Key::D, false, false, false } }, "d.wav" },
    { Key::D, true, false, false, { { Key::D, true, false, false } }, "d.wav" },
    { Key::D, false, true, false, { { Key::D } }, "t.wav" },  // Not a mistake: Ctrl+D should give 'd' with sound 't'
    { Key::E, false, false, false, { { Key::E, false, false, false } }, "e.wav" },
    { Key::E, true, false, false, { { Key::E, true, false, false } }, "e.wav" },
    { Key::E, false, false, true, { { Key::E, false, false, false } }, "ee.wav" },
    { Key::E, false, true, false, { { Key::E } }, "u.wav" },  // Not a mistake: Ctrl+E should give 'e' with sound 'u'
    { Key::F, false, false, false, { { Key::F, false, false, false } }, "f.wav" },
    { Key::F, true, false, false, { { Key::F, true, false, false } }, "f.wav" },
    { Key::G, false, false, false, { { Key::G, false, false, false } }, "g.wav" },
    { Key::G, true, false, false, { { Key::G, true, false, false } }, "g.wav" },
    { Key::H, false, false, false, { { Key::H, false, false, false } }, "h.wav" },
    { Key::H, true, false, false, { { Key::H, true, false, false } }, "h.wav" },
    { Key::I, false, false, false, { { Key::I, false, false, false } }, "i.wav" },
    { Key::I, true, false, false, { { Key::I, true, false, false } }, "i.wav" },
    { Key::I, false, false, true, { { Key::I, false, false, false } }, "ie.wav" },
    { Key::I, false, true, false, { { Key::I } }, "u.wav" },  // Not a mistake: Ctrl+I should give 'i' with sound 'u'
    { Key::J, false, false, false, { { Key::J, false, false, false } }, "j.wav" },
    { Key::J, true, false, false, { { Key::J, true, false, false } }, "j.wav" },
    { Key::K, false, false, false, { { Key::K, false, false, false } }, "k.wav" },
    { Key::K, true, false, false, { { Key::K, true, false, false } }, "k.wav" },
    { Key::L, false, false, false, { { Key::L, false, false, false } }, "l.wav" },
    { Key::L, true, false, false, { { Key::L, true, false, false } }, "l.wav" },
    { Key::M, false, false, false, { { Key::M, false, false, false } }, "m.wav" },
    { Key::M, true, false, false, { { Key::M, true, false, false } }, "m.wav" },
    { Key::N, false, false, false, { { Key::N, false, false, false } }, "n.wav" },
    { Key::N, true, false, false, { { Key::N, true, false, false } }, "n.wav" },
    { Key::O, false, false, false, { { Key::O, false, false, false } }, "o.wav" },
    { Key::O, true, false, false, { { Key::O, true, false, false } }, "o.wav" },
    { Key::O, false, false, true, { { Key::O, false, false, false } }, "oo.wav" },
    { Key::P, false, false, false, { { Key::P, false, false, false } }, "p.wav" },
    { Key::P, true, false, false, { { Key::P, true, false, false } }, "p.wav" },
    { Key::Q, false, false, false, { { Key::Q, false, false, false } }, "q.wav" },
    { Key::Q, true, false, false, { { Key::Q, true, false, false } }, "q.wav" },
    { Key::R, false, false, false, { { Key::R, false, false, false } }, "r.wav" },
    { Key::R, true, false, false, { { Key::R, true, false, false } }, "r.wav" },
    { Key::S, false, false, false, { { Key::S, false, false, false } }, "s.wav" },
    { Key::S, true, false, false, { { Key::S, true, false, false } }, "s.wav" },
    { Key::T, false, false, false, { { Key::T, false, false, false } }, "t.wav" },
    { Key::T, true, false, false, { { Key::T, true, false, false } }, "t.wav" },
    { Key::U, false, false, false, { { Key::U, false, false, false } }, "u.wav" },
    { Key::U, true, false, false, { { Key::U, true, false, false } }, "u.wav" },
    { Key::U, false, false, true, { { Key::U, false, false, false } }, "uu.wav" },
    { Key::V, false, false, false, { { Key::V, false, false, false } }, "v.wav" },
    { Key::V, true, false, false, { { Key::V, true, false, false } }, "v.wav" },
    { Key::W, false, false, false, { { Key::W, false, false, false } }, "w.wav" },
    { Key::W, true, false, false, { { Key::W, true, false, false } }, "w.wav" },
    { Key::X, false, false, false, { { Key::X, false, false, false } }, "x.wav" },
    { Key::X, true, false, false, { { Key::X, true, false, false } }, "x.wav" },
    { Key::Y, false, false, false, { { Key::Y, false, false, false } }, "y.wav" },
    { Key::Y, true, false, false, { { Key::Y, true, false, false } }, "y.wav" },
    { Key::Y, false, true, false, { { Key::Y } }, "i.wav", false, CapsLock::Ignore },  // Not a mistake: Ctrl+Y should give 'y' with sound 'i'
    { Key::Y, false, false, true, { { Key::Y } }, "j.wav", false, CapsLock::Ignore },
    { Key::Z, false, false, false, { { Key::Z, false, false, false } }, "z.wav" },
    { Key::Z, true, false, false, { { Key::Z, true, false, false } }, "z.wav" },
    { Key::X, false, true, false, { { Key::X, false, true, false } } },
    { Key::V, false, true, false, { { Key::V, false, true, false } } , "", false, CapsLock::Ignore },
    { Key::Z, false, true, false, { { Key::Z, false, true, false } } , "", false, CapsLock::Ignore },
    { Key::A, false, true, false, { { Key::A, false, true, false } } , "", false, CapsLock::Ignore },
    { Key::F, false, true, false, { { Key::F, false, true, false } } , "", false, CapsLock::Ignore },
    { Key::Tab, false, false, true, { { Key::Tab, false, false, true } } , "", false, CapsLock::Ignore },
    { Key::F4, false, false, true, { { Key::F4, false, false, true } } , "", false, CapsLock::Ignore },
};

// Layout provider implementation
class DutchDefaultLayout : public Dyscover::ILayoutProvider {
public:
    const std::vector<KeyTranslationEntry>& GetEntries() const override {
        return g_dutchDefault;
    }
    const char* GetName() const override { return "dutchdefault"; }
    const char* GetLanguage() const override { return "nl_nl"; }
};

// Static registration
static DutchDefaultLayout s_layout;
static bool s_registered = []() {
    Dyscover::LayoutRegistry::Instance().Register("dutchdefault", &s_layout);
    return true;
}();

} // namespace
