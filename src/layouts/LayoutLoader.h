// Layout Loader Interface
// Feature: 006-language-resource-optimization (Phase 2)
// Purpose: Abstract interface for loading keyboard layouts at compile time
//
// See: specs/006-language-resource-optimization/contracts/layout-api.md

#pragma once

#include <vector>
#include <map>
#include <string>

#include "LayoutTypes.h"

namespace Dyscover {

/**
 * @brief Abstract interface for layout providers
 * 
 * Each layout module (e.g., res/layouts/classic/nl_nl/layout.cpp) implements
 * this interface to provide layout data to the application.
 */
class ILayoutProvider {
public:
    virtual ~ILayoutProvider() = default;
    
    /**
     * @brief Get the keyboard layout translation entries
     * @return Reference to vector of KeyTranslationEntry objects
     */
    virtual const std::vector<KeyTranslationEntry>& GetEntries() const = 0;
    
    /**
     * @brief Get the layout name (e.g., "dutch_classic")
     * @return Layout name string
     */
    virtual const char* GetName() const = 0;
    
    /**
     * @brief Get the language code (e.g., "nl", "nl_be")
     * @return Language code string
     */
    virtual const char* GetLanguage() const = 0;
};

// TODO: Implement LayoutRegistry class (see LayoutRegistry.h)

} // namespace Dyscover
