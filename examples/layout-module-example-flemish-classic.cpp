// Example Layout Module: Flemish Classic
// Feature: 006-language-resource-optimization (Phase 2)
// This is an EXAMPLE of what the migration script will generate
// Actual file will be created at: res/layouts/classic/nl_be/layout.cpp

#include "Keys.h"
#include "layouts/LayoutLoader.h"
#include "layouts/LayoutRegistry.h"

namespace {

// Layout data (extracted from Keys.cpp g_flemishClassic)
static const std::vector<KeyTranslationEntry> g_flemishClassic = {
    // NOTE: This is abbreviated for example purposes
    // The migration script will extract the full definition
    { Key::A, false, false, false, { { Key::A, false, false, false } }, "a.wav" },
    { Key::B, false, false, false, { { Key::B, false, false, false } }, "b.wav" },
    { Key::C, false, false, false, { { Key::C, false, false, false } }, "c.wav" },
    // ... more entries would be here in the real file
};

// Layout provider implementation
class FlemishClassicLayout : public Dyscover::ILayoutProvider {
public:
    const std::vector<KeyTranslationEntry>& GetEntries() const override {
        return g_flemishClassic;
    }
    
    const char* GetName() const override { 
        return "flemish_classic"; 
    }
    
    const char* GetLanguage() const override { 
        return "nl_be"; 
    }
};

// Static registration (runs before main())
static FlemishClassicLayout s_layout;
static bool s_registered = []() {
    Dyscover::LayoutRegistry::Instance().Register("flemish_classic", &s_layout);
    return true;
}();

} // anonymous namespace
