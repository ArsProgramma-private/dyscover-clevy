#if defined(__linux__)
// DeviceDetectorLinux.cpp - platform-specific device enumeration using libudev

#include "DeviceDetector.h"
#include "SupportedDevices.h"
#include <libudev.h>
#include <string>

class DeviceDetectorLinux : public IDeviceDetector {
public:
    explicit DeviceDetectorLinux(IDeviceDetectorListener* listener) : m_listener(listener) {}
    bool isPresent() const override {
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
    void stopMonitoring() override { /* noop for now */ }
    void refresh() override { if (m_listener) m_listener->onDevicePresenceChanged(isPresent()); }
    PlatformCapabilities capabilities() const override { return 1 << 0; /* HOTPLUG_EVENTS for Linux */ }

private:
    IDeviceDetectorListener* m_listener{nullptr};
};

// Factory for platform builds
std::unique_ptr<IDeviceDetector> CreatePlatformDeviceDetector(IDeviceDetectorListener* listener) {
    return std::unique_ptr<IDeviceDetector>(new DeviceDetectorLinux(listener));
}

#endif // __linux__
#if defined(__PLATFORM_LINUX__)

#include "DeviceDetector.h"
#include "../SupportedDevices.h"
#include <libudev.h>
#include <string>

namespace {

class LinuxDeviceDetector : public IDeviceDetector {
public:
    explicit LinuxDeviceDetector(IDeviceDetectorListener* listener) : m_listener(listener) {}

    bool isPresent() const override {
        // Enumerate USB devices using libudev and match against SupportedDevices
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
                    if (IsSupported(v, p)) {
                        found = true;
                        udev_device_unref(dev);
                        break;
                    }
                }
                udev_device_unref(dev);
            }
        }

        udev_enumerate_unref(enumerate);
        udev_unref(udev);
        return found;
    }

    void startMonitoring() override {
        // Minimal implementation for tests: notify listener of current state once
        if (m_listener) {
            m_listener->onDevicePresenceChanged(isPresent());
        }
    }
    void stopMonitoring() override { /* no-op */ }
    void refresh() override { /* no-op */ }
    PlatformCapabilities capabilities() const override { return 1 << 0; /* HOTPLUG_EVENTS */ }

private:
    IDeviceDetectorListener* m_listener{nullptr};
};

} // anonymous

std::unique_ptr<IDeviceDetector> CreateDeviceDetector_Linux(IDeviceDetectorListener* listener) {
    return std::unique_ptr<IDeviceDetector>(new LinuxDeviceDetector(listener));
}

#endif // __PLATFORM_LINUX__
