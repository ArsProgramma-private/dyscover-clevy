// Unit Test: Runtime Layout Switching
// Verifies that LayoutRegistry::SetActiveLayout changes the active provider
// and that TranslateKey reflects the new layout's data.

#include "layouts/LayoutRegistry.h"
#include "layouts/LayoutLoader.h"
#include "layouts/LayoutTypes.h"
#include "Keys.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace Dyscover;

class TestLayoutProvider : public ILayoutProvider {
public:
    TestLayoutProvider(const char* name, const char* lang, const std::string& sound)
        : name_(name), lang_(lang) {
        KeyTranslationEntry e{};
        e.input = Key::A;
        e.shift = false; e.ctrl = false; e.alt = false;
        e.output = {}; // no keystroke output needed for this test
        e.sound = sound;
        e.speak_sentence = false;
        e.capsLock = CapsLock::Ignore;
        entries_.push_back(e);
    }

    const std::vector<KeyTranslationEntry>& GetEntries() const override { return entries_; }
    const char* GetName() const override { return name_; }
    const char* GetLanguage() const override { return lang_; }

private:
    const char* name_;
    const char* lang_;
    std::vector<KeyTranslationEntry> entries_;
};

static std::string translateSound(Key k) {
    KeyTranslation kt = TranslateKey(k, /*caps*/false, /*shift*/false, /*ctrl*/false, /*alt*/false, Layout::Default);
    return kt.sound;
}

int main() {
    std::cout << "Runtime Layout Switching Unit Test" << std::endl;

    // Arrange: register two layouts with different sounds for key 'A'
    TestLayoutProvider layout1("test_layout_one", "nl", "sound_one.wav");
    TestLayoutProvider layout2("test_layout_two", "nl", "sound_two.wav");

    LayoutRegistry& reg = LayoutRegistry::Instance();
    reg.Register("test_layout_one", &layout1);
    reg.Register("test_layout_two", &layout2);

    // Act + Assert: switch to layout one
    reg.SetActiveLayout("test_layout_one");
    std::string s1 = translateSound(Key::A);
    assert(s1 == "sound_one.wav");

    // Act + Assert: switch to layout two
    reg.SetActiveLayout("test_layout_two");
    std::string s2 = translateSound(Key::A);
    assert(s2 == "sound_two.wav");

    std::cout << "  ✓ Switching active layout updates TranslateKey results" << std::endl;
    std::cout << "ALL PASSED" << std::endl;
    return 0;
}
