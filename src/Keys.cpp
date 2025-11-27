// Keys.cpp - Keyboard translation implementation
// Feature 006 Phase 2: Layout-based resource organization
// This file contains only the translation logic; layout data is in res/layouts/

#include "Keys.h"
#include "layouts/LayoutRegistry.h"
#include "layouts/LayoutLoader.h"
#include <vector>

// Type definitions for layout entries (same as in layout files)
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

// Helper function to find translation in layout entries
static KeyTranslation FindTranslation(const std::vector<KeyTranslationEntry>& entries, Key key, bool caps, bool shift, bool ctrl, bool alt)
{
    CapsLock capsLock = caps ? CapsLock::Active : CapsLock::Inactive;

    for (const KeyTranslationEntry& entry : entries)
    {
        if (entry.input == key && (entry.capsLock == CapsLock::Ignore || entry.capsLock == capsLock) && entry.shift == shift && entry.ctrl == ctrl && entry.alt == alt)
        {
            KeyTranslation kt;
            kt.keystrokes = entry.output;
            kt.sound = entry.sound;
            kt.speak_sentence = entry.speak_sentence;
            return kt;
        }
    }

    return KeyTranslation();
}

// Main translation function - uses layout registry
KeyTranslation TranslateKey(Key key, bool caps, bool shift, bool ctrl, bool alt, Layout layout)
{
    // layout parameter is unused - we use LayoutRegistry::GetActiveLayout() instead
    // Kept for API compatibility with existing callers
    (void)layout;
    
    // Use LayoutRegistry to get the active layout
    const Dyscover::ILayoutProvider* activeLayout = Dyscover::LayoutRegistry::Instance().GetActiveLayout();
    if (activeLayout) {
        return FindTranslation(activeLayout->GetEntries(), key, caps, shift, ctrl, alt);
    }
    
    // Fallback to empty translation if no layout found
    return KeyTranslation();
}
