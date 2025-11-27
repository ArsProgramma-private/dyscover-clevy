// Layout Registry Implementation
// Feature: 006-language-resource-optimization (Phase 2)
// Purpose: Central registry for keyboard layouts

#include "LayoutRegistry.h"
#include <iostream>

namespace Dyscover {

LayoutRegistry& LayoutRegistry::Instance() {
    static LayoutRegistry instance;
    return instance;
}

void LayoutRegistry::Register(const char* name, ILayoutProvider* provider) {
    if (!name || !provider) {
        std::cerr << "LayoutRegistry::Register: Invalid arguments (name or provider is null)" << std::endl;
        return;
    }
    
    std::string key(name);
    
    // Check for duplicate registration
    if (layouts_.find(key) != layouts_.end()) {
        std::cerr << "LayoutRegistry::Register: Layout '" << name << "' already registered" << std::endl;
        return;
    }
    
    layouts_[key] = provider;
    
    #ifdef DEBUG_LAYOUT_REGISTRY
    std::cout << "LayoutRegistry: Registered layout '" << name << "'" << std::endl;
    #endif
}

const ILayoutProvider* LayoutRegistry::GetLayout(const char* name) const {
    if (!name) {
        return nullptr;
    }
    
    std::string key(name);
    auto it = layouts_.find(key);
    
    if (it != layouts_.end()) {
        return it->second;
    }
    
    #ifdef DEBUG_LAYOUT_REGISTRY
    std::cerr << "LayoutRegistry::GetLayout: Layout '" << name << "' not found" << std::endl;
    #endif
    
    return nullptr;
}

const ILayoutProvider* LayoutRegistry::GetActiveLayout() const {
    // Determine active layout based on compile-time preprocessor flags
    // Priority order:
    // 1. Specific layout type + language (e.g., __LAYOUT_DUTCH_CLASSIC__)
    // 2. Language default (e.g., __LANGUAGE_NL__)
    // 3. Fallback to first registered layout
    
    #if defined(__LAYOUT_DUTCH_CLASSIC__)
        return GetLayout("dutch_classic");
    #elif defined(__LAYOUT_FLEMISH_CLASSIC__)
        return GetLayout("flemish_classic");
    #elif defined(__LAYOUT_DUTCH_KWEC__)
        return GetLayout("dutch_kwec");
    #elif defined(__LAYOUT_DUTCH_DEFAULT__)
        return GetLayout("dutch_default");
    #elif defined(__LAYOUT_FLEMISH_DEFAULT__)
        return GetLayout("flemish_default");
    #elif defined(__LANGUAGE_NL__)
        // Default to classic Dutch if no specific layout selected
        const ILayoutProvider* layout = GetLayout("dutch_classic");
        if (!layout) layout = GetLayout("dutch_default");
        return layout;
    #elif defined(__LANGUAGE_NL_BE__)
        // Default to classic Flemish if no specific layout selected
        const ILayoutProvider* layout = GetLayout("flemish_classic");
        if (!layout) layout = GetLayout("flemish_default");
        return layout;
    #else
        // Fallback: return first registered layout
        if (!layouts_.empty()) {
            #ifdef DEBUG_LAYOUT_REGISTRY
            std::cerr << "LayoutRegistry::GetActiveLayout: No specific layout selected, using first registered: " 
                     << layouts_.begin()->first << std::endl;
            #endif
            return layouts_.begin()->second;
        }
        return nullptr;
    #endif
}

} // namespace Dyscover
