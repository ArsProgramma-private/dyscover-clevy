// PronunciationModifierTest.cpp - Verify Alt key produces alternate pronunciations
#include "Keys.h"
#include "layouts/LayoutRegistry.h"
#include "layouts/LayoutLoader.h"
#include "layouts/LayoutTypes.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace Dyscover;

// Mock layout provider with Alt-modified entries
class TestLayoutWithAlt : public ILayoutProvider {
public:
    TestLayoutWithAlt() {
        // Entry without Alt: Key::A → sound "a.wav"
        KeyTranslationEntry e1{};
        e1.input = Key::A;
        e1.shift = false; e1.ctrl = false; e1.alt = false;
        e1.output = {};
        e1.sound = "a.wav";
        e1.speak_sentence = false;
        e1.capsLock = CapsLock::Ignore;
        entries_.push_back(e1);

        // Entry with Alt: Key::A + Alt → sound "aa.wav"
        KeyTranslationEntry e2{};
        e2.input = Key::A;
        e2.shift = false; e2.ctrl = false; e2.alt = true;
        e2.output = {};
        e2.sound = "aa.wav";
        e2.speak_sentence = false;
        e2.capsLock = CapsLock::Ignore;
        entries_.push_back(e2);
    }

    const std::vector<KeyTranslationEntry>& GetEntries() const override { return entries_; }
    const char* GetName() const override { return "test_alt_modifier"; }
    const char* GetLanguage() const override { return "nl"; }

private:
    std::vector<KeyTranslationEntry> entries_;
};

int main() {
    std::cout << "Pronunciation Modifier (Alt Key) Unit Test" << std::endl;

    // Register test layout
    TestLayoutWithAlt layout;
    LayoutRegistry::Instance().Register("test_alt_modifier", &layout);
    LayoutRegistry::Instance().SetActiveLayout("test_alt_modifier");

    // Test: Alt+A should produce "aa.wav" instead of "a.wav"
    KeyTranslation noAlt = TranslateKey(Key::A, false, false, false, false, Layout::Default);
    KeyTranslation withAlt = TranslateKey(Key::A, false, false, false, true, Layout::Default);

    std::cout << "  Without Alt: sound=" << noAlt.sound << std::endl;
    std::cout << "  With Alt:    sound=" << withAlt.sound << std::endl;

    assert(noAlt.sound == "a.wav");
    assert(withAlt.sound == "aa.wav");

    std::cout << "  ✓ Alt modifier changes pronunciation (A → aa)" << std::endl;
    std::cout << "ALL PASSED" << std::endl;
    return 0;
}
