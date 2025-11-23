// DeviceHotplugCallbackTest.cpp - Failing skeleton for T021
#include "platform/DeviceDetector.h"
#include <iostream>

class DummyListener : public IDeviceDetectorListener {
public:
    void onDevicePresenceChanged(bool present) override { lastSeen = present; }
    bool lastSeen{false};
};

int main() {
    DummyListener l;
    auto detector = CreateDeviceDetector(&l);

    // We expect hotplug capability in future - test will fail until implemented
    if((detector->capabilities() & 1) == 0) {
        std::cerr << "Expected HOTPLUG_EVENTS capability to be set" << std::endl;
        return 1;
    }

    // start monitoring should register callbacks to the listener
    detector->startMonitoring();

    // simulate callback: we expect the detector to call listener (will not yet)
    if(!l.lastSeen) {
        std::cerr << "Expected listener to be invoked on hotplug (will implement)" << std::endl;
        return 2;
    }

    std::cout << "DeviceHotplugCallbackTest executed (expected failures)" << std::endl;
    return 0;
}
