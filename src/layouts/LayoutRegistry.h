// Layout Registry
// Feature: 006-language-resource-optimization (Phase 2)
// Purpose: Central registry for compile-time layout selection
//
// See: specs/006-language-resource-optimization/contracts/layout-api.md

#pragma once

#include "LayoutLoader.h"
#include <map>
#include <string>

namespace Dyscover {

/**
 * @brief Central registry for keyboard layouts
 * 
 * Layouts register themselves at static initialization time.
 * The active layout is selected at compile time via preprocessor flags.
 * 
 * Usage:
 *   // In layout module static initializer:
 *   LayoutRegistry::Instance().Register("dutch_classic", &myLayout);
 * 
 *   // In application code:
 *   auto* layout = LayoutRegistry::Instance().GetActiveLayout();
 */
class LayoutRegistry {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to global LayoutRegistry
     */
    static LayoutRegistry& Instance();
    
    /**
     * @brief Register a layout provider
     * @param name Layout name (e.g., "dutch_classic")
     * @param provider Pointer to ILayoutProvider implementation
     */
    void Register(const char* name, ILayoutProvider* provider);
    
    /**
     * @brief Get layout by name
     * @param name Layout name to retrieve
     * @return Pointer to layout provider, or nullptr if not found
     */
    const ILayoutProvider* GetLayout(const char* name) const;
    
    /**
     * @brief Get currently active layout (compile-time selected)
     * @return Pointer to active layout provider
     */
    const ILayoutProvider* GetActiveLayout() const;
    
private:
    LayoutRegistry() = default;
    LayoutRegistry(const LayoutRegistry&) = delete;
    LayoutRegistry& operator=(const LayoutRegistry&) = delete;
    
    // TODO: Implement layout storage (map of name -> provider)
    std::map<std::string, ILayoutProvider*> layouts_;
};

} // namespace Dyscover
