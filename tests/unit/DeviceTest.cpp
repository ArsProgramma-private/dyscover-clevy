//
// DeviceTest.cpp
//

#include "DeviceConfig.h"
#include <cassert>
#include <iostream>

void testDeviceConfig() {
    DeviceConfig config;

    // Test default devices
    config.load("nonexistent.json"); // Should create defaults
    assert(config.getDevices().size() == 2);

    // Test adding device
    SupportedDevice newDev("ABCD", "EF01", "Test Device");
    assert(config.addDevice(newDev));
    assert(config.isDeviceSupported("ABCD", "EF01"));

    // Test duplicate
    assert(!config.addDevice(newDev));

    std::cout << "testDeviceConfig passed" << std::endl;
}

int main() {
    testDeviceConfig();
    std::cout << "All DeviceTest tests passed!" << std::endl;
    return 0;
}