// TrayIconMappingTest.cpp - Validate icon index selection for state permutations
// Tests the mapping logic from TrayIcon::UpdateIcon() in isolation

#include <iostream>
#include <cassert>

// Extracted mapping logic from TrayIcon::UpdateIcon
int SelectIconIndex(bool enabled, bool kbPresent, bool isFullBuild) {
    int iconIndex = 0; // Active by default
    if (isFullBuild) {
        if (!enabled) {
            iconIndex = 4; // Paused
        } else if (!kbPresent) {
            iconIndex = 5; // No Keyboard
        } else {
            iconIndex = 0; // Active
        }
    } else {
        iconIndex = enabled ? 0 : 4; // Active vs Paused
    }
    // Guard against out-of-range (not tested here; always returns 0-5)
    return iconIndex;
}

int main() {
    std::cout << "TrayIcon Mapping Unit Test" << std::endl;

    // Demo build: only enabled/paused states
    assert(SelectIconIndex(true, true, false) == 0);   // Active
    assert(SelectIconIndex(true, false, false) == 0);  // Active (kb ignored)
    assert(SelectIconIndex(false, true, false) == 4);  // Paused
    assert(SelectIconIndex(false, false, false) == 4); // Paused

    // Full build: all three states
    assert(SelectIconIndex(true, true, true) == 0);    // Active
    assert(SelectIconIndex(true, false, true) == 5);   // No Keyboard
    assert(SelectIconIndex(false, true, true) == 4);   // Paused (takes precedence)
    assert(SelectIconIndex(false, false, true) == 4);  // Paused

    std::cout << "  ✓ All icon state mappings correct" << std::endl;
    std::cout << "ALL PASSED" << std::endl;
    return 0;
}
