//
// ConfigTest.cpp
//

#include "DeviceConfig.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>

void testHexValidation() {
    DeviceConfig config;
    SupportedDevice valid("04B4", "0101");
    assert(config.validateDevice(valid));

    SupportedDevice invalidVid("ZZZZ", "0101");
    assert(!config.validateDevice(invalidVid));

    SupportedDevice invalidPid("04B4", "ZZZZ");
    assert(!config.validateDevice(invalidPid));

    SupportedDevice shortVid("4B4", "0101");
    assert(!config.validateDevice(shortVid));

    std::cout << "testHexValidation passed" << std::endl;
}

void testDuplicateDetection() {
    DeviceConfig config;
    SupportedDevice dev1("04B4", "0101");
    SupportedDevice dev2("04B4", "0101");

    assert(config.addDevice(dev1));
    assert(!config.addDevice(dev2)); // Should fail due to duplicate

    std::cout << "testDuplicateDetection passed" << std::endl;
}

void testConfigLoadSave() {
    // Create temp file using C++17 filesystem (cross-platform)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10000, 99999);
    
    auto tempDir = std::filesystem::temp_directory_path();
    auto tempFile = tempDir / ("test_config_" + std::to_string(dis(gen)) + ".json");
    std::string tempFilePath = tempFile.string();

    {
        DeviceConfig config;
        config.load(tempFilePath); // Should create default
        assert(config.getDevices().size() == 2); // Default devices
    }

    {
        DeviceConfig config;
        SupportedDevice newDev("1234", "5678", "Test Device");
        config.addDevice(newDev);
        config.save(tempFilePath);
    }

    {
        DeviceConfig config;
        config.load(tempFilePath);
        assert(config.getDevices().size() == 3); // 2 default + 1 added
        assert(config.isDeviceSupported("1234", "5678"));
    }

    std::filesystem::remove(tempFile);
    std::cout << "testConfigLoadSave passed" << std::endl;
}

int main() {
    testHexValidation();
    testDuplicateDetection();
    testConfigLoadSave();
    std::cout << "All ConfigTest tests passed!" << std::endl;
    return 0;
}