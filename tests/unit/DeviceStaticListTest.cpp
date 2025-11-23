// Placeholder unit tests for static supported device list.
// Failing tests will be added before implementing matching logic (Red-Green-Refactor).
// Accessibility note: Any output destined for UI should later ensure semantic labeling.

#include "../../src/SupportedDevices.h"
#include <cassert>
#include <iostream>

static void testNormalize() {
    assert(NormalizeHex4("04b4") == "04B4");
    assert(NormalizeHex4("4b4") == "04B4"); // left-padded with zero
    assert(NormalizeHex4("04B4AA") == "04B4"); // truncated
}

static void testIsSupportedPositive() {
    assert(IsSupported("04B4", "0101") == true);
    assert(IsSupported("04b4", "0101") == true); // case normalization
}

static void testIsSupportedNegative() {
    assert(IsSupported("FFFF", "FFFF") == false);
    assert(IsSupported("0000", "0101") == false);
}

int main() {
    testNormalize();
    testIsSupportedPositive();
    testIsSupportedNegative();
    std::cout << "All static list tests passed.\n";
    return 0;
}
