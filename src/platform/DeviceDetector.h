#pragma once
#include <memory>
#include <cstdint>
#include <string>

class IDeviceDetectorListener {
public:
    virtual void onDevicePresenceChanged(bool present) = 0;
    virtual ~IDeviceDetectorListener() = default;
};

using PlatformCapabilities = uint32_t;

class IDeviceDetector {
public:
    virtual bool isPresent() const = 0;
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
    virtual void refresh() = 0;
    virtual PlatformCapabilities capabilities() const = 0;
    virtual ~IDeviceDetector() = default;
};

std::unique_ptr<IDeviceDetector> CreateDeviceDetector(IDeviceDetectorListener* listener);
