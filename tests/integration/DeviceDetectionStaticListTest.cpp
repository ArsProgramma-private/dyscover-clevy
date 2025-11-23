// Integration test simulation for static device list detection.
// This does not enumerate real hardware in this environment; instead it
// validates consistency of IsSupported logic across representative cases.
// Accessibility: Any surfaced test results in UI should use clear textual status.

#include "../../src/SupportedDevices.h"
#include <cassert>
#include <iostream>
#include <vector>

struct ProbeCase { std::string vid; std::string pid; bool expected; };

int main() {
    std::vector<ProbeCase> cases = {
        {"04B4", "0101", true},      // exact
        {"04b4", "0101", true},      // lowercase VID
        {"04B4", "0101 ", true},     // trailing space
        {"FFFF", "0101", false},     // unsupported VID
        {"04B4", "FFFF", false},     // unsupported PID
        {"0000", "0000", false}      // definitely unsupported
    };

    for (const auto& c : cases) {
        bool supported = IsSupported(c.vid, c.pid);
        if (supported != c.expected) {
            std::cerr << "Mismatch for VID=" << c.vid << " PID=" << c.pid
                      << " expected=" << c.expected << " got=" << supported << std::endl;
            assert(false);
        }
    }

    std::cout << "Integration simulation passed (static list).\n";
    return 0;
}
