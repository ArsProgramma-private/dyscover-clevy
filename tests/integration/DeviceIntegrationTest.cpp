//
// DeviceIntegrationTest.cpp
//

#include "DeviceConfig.h"
#include <cassert>
#include <iostream>
#include <fstream>

void testConfigIntegration() {
    // Test loading and using config in device-like scenario
    DeviceConfig config;
    config.load("test_config.json");

    // Simulate device detection
    std::string testVid = "04B4";
    std::string testPid = "0101";

    bool supported = config.isDeviceSupported(testVid, testPid);
    assert(supported); // Should be in defaults

    // Add new device
    SupportedDevice newDev("FFFF", "0000");
    config.addDevice(newDev);

    assert(config.isDeviceSupported("FFFF", "0000"));

    std::cout << "testConfigIntegration passed" << std::endl;
}

int main() {
    testConfigIntegration();
    std::cout << "All DeviceIntegrationTest tests passed!" << std::endl;
    return 0;
}