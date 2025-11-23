// DeviceHotplugCallbackTest.cpp - Test for hotplug callback mechanism (T026)
#include "platform/DeviceDetector.h"
#include <iostream>

class MockListener : public IDeviceDetectorListener {
public:
    void onDevicePresenceChanged(bool present) override { 
        lastPresent = present;
        callbackCount++;
    }
    bool lastPresent{false};
    int callbackCount{0};
};

int main() {
    MockListener listener;
    auto detector = CreateDeviceDetector(&listener);

    // T026: Test hotplug event callback mechanism
    // Verify HOTPLUG_EVENTS capability flag (bit 0) is set by platform implementation
    const int HOTPLUG_EVENTS = 1;
    if((detector->capabilities() & HOTPLUG_EVENTS) == 0) {
        std::cerr << "FAIL: Expected HOTPLUG_EVENTS capability flag (bit 0)" << std::endl;
        return 1;
    }

    // Start monitoring should activate hotplug detection and trigger initial callback
    detector->startMonitoring();

    // Platform implementation should call listener with initial state on startMonitoring()
    int initialCallbackCount = listener.callbackCount;
    if(initialCallbackCount == 0) {
        std::cerr << "FAIL: Expected initial callback on startMonitoring()" << std::endl;
        return 2;
    }

    // Verify initial state was reported (should be false - no device connected in test environment)
    if(listener.lastPresent != false) {
        std::cerr << "FAIL: Expected initial presence = false (no device in test environment)" << std::endl;
        return 3;
    }

    // Reset counter to verify refresh() behavior with no state change
    listener.callbackCount = 0;
    
    // Call refresh() - should re-enumerate devices but NOT trigger callback
    // because the state hasn't changed (still no device present)
    detector->refresh();
    
    if(listener.callbackCount != 0) {
        std::cerr << "FAIL: refresh() triggered callback without state change" << std::endl;
        return 4;
    }

    // Test passes - hotplug mechanism is working correctly:
    // - Initial callback on startMonitoring() ✓
    // - No spurious callbacks when state unchanged ✓
    // - Detector reports HOTPLUG_EVENTS capability ✓
    
    detector->stopMonitoring();

    std::cout << "DeviceHotplugCallbackTest PASS - hotplug detection works correctly" << std::endl;
    return 0;
}
