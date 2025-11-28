// Layout Types
// Shared definitions for keyboard layout entries used by LayoutRegistry providers

#pragma once

#include <vector>
#include <string>

#include "../Keys.h" // for Key and KeyStroke

namespace Dyscover {

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

} // namespace Dyscover
