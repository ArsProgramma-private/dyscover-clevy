// DeviceDetectorHotplugMockTest.cpp - Failing skeleton for T026
#include "platform/DeviceDetector.h"
#include <iostream>

class MockListener : public IDeviceDetectorListener {
public:
    void onDevicePresenceChanged(bool present) override { seen = present; }
    bool seen{false};
};

int main() {
    MockListener l;
    auto detector = CreateDeviceDetector(&l);

    // Expect that startMonitoring will fire callback at least once in the future
    detector->startMonitoring();
    if(!l.seen) {
        std::cerr << "Expected listener to be notified during monitoring (will implement)" << std::endl;
        return 1;
    }

    detector->stopMonitoring();

    std::cout << "DeviceDetectorHotplugMockTest executed (expected failures)" << std::endl;
    return 0;
}
