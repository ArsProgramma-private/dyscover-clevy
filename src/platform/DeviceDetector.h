#pragma once
#include <memory>
#include <cstdint>
#include <string>

class IDeviceDetectorListener {
public:
    virtual void onDevicePresenceChanged(bool present) = 0;
    virtual ~IDeviceDetectorListener() = default;
};

// Platform capability flags for device detection
using PlatformCapabilities = uint32_t;

// Capability flag: Platform supports native hotplug event notifications
// (e.g., Windows WM_DEVICECHANGE, Linux udev monitor, macOS IOKit notifications)
constexpr PlatformCapabilities HOTPLUG_EVENTS = 1 << 0;

// Capability flag: Platform uses polling to simulate hotplug events
// (set by PollingDeviceDetector wrapper when native hotplug unavailable)
constexpr PlatformCapabilities POLLING_ONLY = 1 << 2;

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
