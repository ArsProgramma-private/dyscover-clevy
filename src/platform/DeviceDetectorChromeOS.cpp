#if defined(__linux__)
// DeviceDetectorChromeOS.cpp - ChromeOS variant: reuse libudev when available otherwise provide polling-only fallback

#include "DeviceDetector.h"
#include "SupportedDevices.h"
#include <libudev.h>
#include <string>

class DeviceDetectorChromeOS : public IDeviceDetector {
public:
    explicit DeviceDetectorChromeOS(IDeviceDetectorListener* listener) : m_listener(listener) {}

    bool isPresent() const override {
        // On ChromeOS we may be sandboxed; attempt udev first
        struct udev* udev = udev_new();
        if (!udev) return false;

        struct udev_enumerate* enumerate = udev_enumerate_new(udev);
        if (!enumerate) { udev_unref(udev); return false; }

        udev_enumerate_add_match_subsystem(enumerate, "usb");
        udev_enumerate_scan_devices(enumerate);

        struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
        struct udev_list_entry* entry;
        bool found = false;

        udev_list_entry_foreach(entry, devices) {
            const char* path = udev_list_entry_get_name(entry);
            struct udev_device* dev = udev_device_new_from_syspath(udev, path);
            if (dev) {
                const char* vid = udev_device_get_sysattr_value(dev, "idVendor");
                const char* pid = udev_device_get_sysattr_value(dev, "idProduct");
                if (vid && pid) {
                    std::string v(vid), p(pid);
                    if (IsSupported(v, p)) { found = true; udev_device_unref(dev); break; }
                }
                udev_device_unref(dev);
            }
        }

        udev_enumerate_unref(enumerate);
        udev_unref(udev);
        return found;
    }

    void startMonitoring() override { if (m_listener) m_listener->onDevicePresenceChanged(isPresent()); }
    void stopMonitoring() override { /* noop */ }
    void refresh() override { if (m_listener) m_listener->onDevicePresenceChanged(isPresent()); }
    PlatformCapabilities capabilities() const override { return 1 << 2; /* POLLING_ONLY by default on ChromeOS if sandboxed */ }

private:
    IDeviceDetectorListener* m_listener{nullptr};
};

#endif // __linux__

// Note: Do NOT provide CreatePlatformDeviceDetector here by default because
// the regular Linux detector provides the platform factory for Linux builds.
// ChromeOS-specific selection will be wired into the platform factory once
// a build-time ChromeOS marker is available.
