// DeviceDetectorTest.cpp - Failing skeleton for T017
#include "platform/DeviceDetector.h"
#include <cassert>
#include <iostream>

class TestListener : public IDeviceDetectorListener {
public:
    void onDevicePresenceChanged(bool present) override { lastPresent = present; }
    bool lastPresent{false};
};

// Expectations (initially failing):
// 1. Detector reports not implemented presence semantics (we expect true later)
// 2. Capabilities should advertise at least one non-zero flag later
// 3. Monitoring should be callable without crash
int main() {
    TestListener listener;
    auto detector = CreateDeviceDetector(&listener);

    // Failing expectations (will be updated once implementation exists)
    // Expect presence to be true for connected mock device later
    if(detector->isPresent() != true) {
        std::cerr << "Expected initial isPresent() to be true (will implement)" << std::endl;
        return 1; // fail
    }
    if(detector->capabilities() == 0) {
        std::cerr << "Expected capabilities to be non-zero (HOTPLUG/POLLING)" << std::endl;
        return 2; // fail
    }

    // Start/stop should not throw or crash
    detector->startMonitoring();
    detector->stopMonitoring();

    std::cout << "DeviceDetectorTest skeleton executed (expected failures)" << std::endl;
    return 0; // NOTE: returning 0 still marks test pass for ctest; consider integrating gtest later
}
