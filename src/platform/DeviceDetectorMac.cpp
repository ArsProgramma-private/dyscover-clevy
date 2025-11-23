#if defined(__APPLE__)
// DeviceDetectorMac.cpp - macOS device detector placeholder (IOKit based)

#include "DeviceDetector.h"
#include "SupportedDevices.h"
#include <string>
#include <IOKit/IOKitLib.h>

class DeviceDetectorMac : public IDeviceDetector {
public:
    explicit DeviceDetectorMac(IDeviceDetectorListener* listener) : m_listener(listener) {}

    bool isPresent() const override {
        // Lightweight placeholder: in-depth IOKit enumeration will be implemented later
        return false;
    }

    void startMonitoring() override { if (m_listener) m_listener->onDevicePresenceChanged(isPresent()); }
    void stopMonitoring() override { /* noop */ }
    void refresh() override { if (m_listener) m_listener->onDevicePresenceChanged(isPresent()); }
    PlatformCapabilities capabilities() const override { return 0; }

private:
    IDeviceDetectorListener* m_listener{nullptr};
};

#endif // __APPLE__

#if defined(__APPLE__)
std::unique_ptr<IDeviceDetector> CreatePlatformDeviceDetector(IDeviceDetectorListener* listener) {
    return std::unique_ptr<IDeviceDetector>(new DeviceDetectorMac(listener));
}
#endif
