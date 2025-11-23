// DeviceHotplugCallbackTest.cpp - Extended failing test for T026 (hotplug mock)
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
    // Expect HOTPLUG_EVENTS capability flag (bit 0) to be set once implemented
    const int HOTPLUG_EVENTS = 1;
    if((detector->capabilities() & HOTPLUG_EVENTS) == 0) {
        std::cerr << "RED: Expected HOTPLUG_EVENTS capability flag (bit 0)" << std::endl;
        return 1;
    }

    // Start monitoring should activate hotplug detection
    detector->startMonitoring();

    // In a real implementation, we'd simulate a device connection event.
    // For this unit test, we verify the listener can be called.
    // The stub may call the callback on startMonitoring (initial state notification),
    // which is acceptable. Reset counter to test refresh() behavior specifically.
    
    int initialCallbackCount = listener.callbackCount;
    listener.callbackCount = 0; // Reset to test refresh() independently
    
    // Mock scenario: if implementation were complete, refresh() would re-enumerate
    // devices and trigger onDevicePresenceChanged if state changed.
    // The stub's refresh() is a no-op, so this should NOT increment callbackCount.
    detector->refresh();
    
    if(listener.callbackCount == 0) {
        std::cerr << "RED: Expected listener callback on refresh (detection not implemented)" << std::endl;
        std::cerr << "     Initial callbacks on startMonitoring: " << initialCallbackCount << std::endl;
        return 2;
    }

    // Verify callback was invoked with correct parameters
    // (In stub: no callback = test fails here)
    
    detector->stopMonitoring();

    std::cout << "DeviceHotplugCallbackTest unexpectedly passed (hotplug implemented)" << std::endl;
    return 0;
}
