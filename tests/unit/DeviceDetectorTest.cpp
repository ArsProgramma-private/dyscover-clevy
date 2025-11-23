// DeviceDetectorTest.cpp - Extended failing test for T025 (VID/PID match)
#include "platform/DeviceDetector.h"
#include "SupportedDevices.h"
#include <cassert>
#include <iostream>

class TestListener : public IDeviceDetectorListener {
public:
    void onDevicePresenceChanged(bool present) override { 
        lastPresent = present;
        callbackCount++;
    }
    bool lastPresent{false};
    int callbackCount{0};
};

int main() {
    TestListener listener;
    auto detector = CreateDeviceDetector(&listener);

    // T025: Test VID/PID match logic - expect detector to identify supported device
    // Current stub returns false; this should fail until detection implemented
    bool present = detector->isPresent();
    
    // For this test we expect the detector to report presence if a supported device
    // (04B4:0101 from SupportedDevices.h) is connected. Since this is a unit test
    // without actual hardware, we expect the stub to return false (RED state).
    // Once implemented with proper Linux/Windows/Mac enumeration, this will detect real devices.
    
    if(present == true) {
        std::cerr << "RED: Unexpected device presence in stub implementation" << std::endl;
        return 1;
    }
    
    // Test that IsSupported helper works correctly with normalized VID/PID
    if(!IsSupported("04B4", "0101")) {
        std::cerr << "RED: IsSupported failed for known device 04B4:0101" << std::endl;
        return 2;
    }
    
    if(!IsSupported("04b4", "101")) { // Test normalization (lowercase, short PID)
        std::cerr << "RED: IsSupported normalization failed" << std::endl;
        return 3;
    }
    
    if(IsSupported("FFFF", "FFFF")) { // Unknown device should return false
        std::cerr << "RED: IsSupported returned true for unknown device" << std::endl;
        return 4;
    }

    // Test capabilities reporting (expect non-zero once implemented)
    if(detector->capabilities() == 0) {
        std::cerr << "RED: Expected capabilities flags to be non-zero (HOTPLUG or POLLING)" << std::endl;
        return 5;
    }

    // Start/stop monitoring should not crash
    detector->startMonitoring();
    detector->stopMonitoring();

    std::cout << "DeviceDetectorTest unexpectedly passed (VID/PID detection implemented)" << std::endl;
    return 0;
}
