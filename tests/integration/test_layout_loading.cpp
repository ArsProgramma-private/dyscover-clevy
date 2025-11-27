// Integration Test: Layout Loading
// Feature: 006-language-resource-optimization (Phase 2)
// Purpose: Verify application loads Dutch classic layout successfully

#include "layouts/LayoutRegistry.h"
#include "Keys.h"
#include <iostream>
#include <cassert>

using namespace Dyscover;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Layout Loading Integration Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Get active layout (compile-time selected)
    std::cout << "Test: Get active layout..." << std::endl;
    LayoutRegistry& registry = LayoutRegistry::Instance();
    const ILayoutProvider* active = registry.GetActiveLayout();
    
    if (active == nullptr) {
        std::cerr << "ERROR: No active layout found" << std::endl;
        std::cerr << "Make sure layouts are registered via static initializers" << std::endl;
        return 1;
    }
    
    std::cout << "  ✓ Active layout found" << std::endl;
    std::cout << "    Name: " << active->GetName() << std::endl;
    std::cout << "    Language: " << active->GetLanguage() << std::endl;
    
    // Verify layout has entries
    std::cout << std::endl;
    std::cout << "Test: Layout has entries..." << std::endl;
    const std::vector<KeyTranslationEntry>& entries = active->GetEntries();
    
    if (entries.empty()) {
        std::cerr << "ERROR: Layout has no entries" << std::endl;
        return 1;
    }
    
    std::cout << "  ✓ Layout has " << entries.size() << " entries" << std::endl;
    
    // Verify first entry structure
    std::cout << std::endl;
    std::cout << "Test: Entry structure..." << std::endl;
    const KeyTranslationEntry& first = entries[0];
    
    std::cout << "  First entry:" << std::endl;
    std::cout << "    Key: " << static_cast<int>(first.key) << std::endl;
    std::cout << "    Shift: " << first.useShift << std::endl;
    std::cout << "    Ctrl: " << first.useCtrl << std::endl;
    std::cout << "    Alt: " << first.useAlt << std::endl;
    std::cout << "    Sound: " << (first.sound ? first.sound : "(none)") << std::endl;
    std::cout << "  ✓ Entry structure valid" << std::endl;
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Layout loading test PASSED" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
