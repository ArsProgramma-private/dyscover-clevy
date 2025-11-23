#if defined(__linux__) && !defined(__ANDROID__)
// DeviceDetectorLinux.cpp - Linux device detection using libudev
// Refactored from DeviceLinux.cpp to support platform abstraction layer

#include "DeviceDetector.h"
#include "../SupportedDevices.h"
#include <libudev.h>
#include <atomic>
#include <thread>
#include <string>
#include <cstring>
#include <chrono>

namespace {
    constexpr PlatformCapabilities HOTPLUG_EVENTS = 1 << 0;

    // Linux device detector implementation using libudev
    class LinuxDeviceDetector : public IDeviceDetector {
    public:
        explicit LinuxDeviceDetector(IDeviceDetectorListener* listener)
            : m_listener(listener),
              m_udev(nullptr),
              m_monitor(nullptr),
              m_monitorFd(-1),
              m_monitoring(false)
        {
            m_udev = udev_new();
            if (!m_udev) {
                // Log error but continue - isPresent() will return false
                return;
            }

            // Initialize device presence state
            refresh();
        }

        ~LinuxDeviceDetector() override {
            stopMonitoring();
            if (m_udev) {
                udev_unref(m_udev);
            }
        }

        bool isPresent() const override {
            return m_present.load();
        }

        void startMonitoring() override {
            if (m_monitoring.load() || !m_udev) return;

            // Create udev monitor for USB subsystem hotplug events
            m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
            if (!m_monitor) {
                // Fallback to basic presence check
                if (m_listener) {
                    m_listener->onDevicePresenceChanged(m_present.load());
                }
                return;
            }

            udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", nullptr);
            udev_monitor_enable_receiving(m_monitor);
            m_monitorFd = udev_monitor_get_fd(m_monitor);

            m_monitoring = true;

            // Notify listener of initial state
            if (m_listener) {
                m_listener->onDevicePresenceChanged(m_present.load());
            }

            // Start monitoring thread
            m_monitorThread = std::thread([this]() {
                while (m_monitoring.load()) {
                    // Poll with timeout to allow graceful shutdown
                    fd_set fds;
                    struct timeval tv;
                    int ret;

                    FD_ZERO(&fds);
                    FD_SET(m_monitorFd, &fds);
                    tv.tv_sec = 0;
                    tv.tv_usec = 500000; // 500ms timeout

                    ret = select(m_monitorFd + 1, &fds, nullptr, nullptr, &tv);
                    
                    if (ret > 0 && FD_ISSET(m_monitorFd, &fds)) {
                        // Device event received - process it
                        struct udev_device* dev = udev_monitor_receive_device(m_monitor);
                        if (dev) {
                            // Check if it's a supported device being added/removed
                            const char* action = udev_device_get_action(dev);
                            if (action && (strcmp(action, "add") == 0 || strcmp(action, "remove") == 0)) {
                                refresh(); // Re-scan device list and notify if changed
                            }
                            udev_device_unref(dev);
                        }
                    }
                }
            });
        }

        void stopMonitoring() override {
            if (!m_monitoring.load()) return;

            m_monitoring = false;

            // Wait for monitoring thread to exit
            if (m_monitorThread.joinable()) {
                m_monitorThread.join();
            }

            // Clean up monitor
            if (m_monitor) {
                udev_monitor_unref(m_monitor);
                m_monitor = nullptr;
                m_monitorFd = -1;
            }
        }

        void refresh() override {
            if (!m_udev) {
                m_present = false;
                return;
            }

            // Enumerate USB devices and check for supported VID/PID
            struct udev_enumerate* enumerate = udev_enumerate_new(m_udev);
            if (!enumerate) {
                return;
            }

            udev_enumerate_add_match_subsystem(enumerate, "usb");
            udev_enumerate_scan_devices(enumerate);

            struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
            struct udev_list_entry* entry;

            bool found = false;
            udev_list_entry_foreach(entry, devices) {
                const char* path = udev_list_entry_get_name(entry);
                struct udev_device* dev = udev_device_new_from_syspath(m_udev, path);
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

            bool oldPresence = m_present.exchange(found);
            
            // Notify listener if state changed and monitoring is active
            if (m_monitoring.load() && m_listener && (oldPresence != found)) {
                m_listener->onDevicePresenceChanged(found);
            }
        }

        PlatformCapabilities capabilities() const override {
            return HOTPLUG_EVENTS; // Linux supports native hotplug via udev monitor
        }

    private:
        IDeviceDetectorListener* m_listener;
        struct udev* m_udev;
        struct udev_monitor* m_monitor;
        int m_monitorFd;
        std::atomic<bool> m_present{false};
        std::atomic<bool> m_monitoring{false};
        std::thread m_monitorThread;
    };

} // anonymous namespace

// Platform-specific factory function for Linux
std::unique_ptr<IDeviceDetector> CreatePlatformDeviceDetector(IDeviceDetectorListener* listener) {
    return std::unique_ptr<IDeviceDetector>(new LinuxDeviceDetector(listener));
}

#endif // __linux__ && !__ANDROID__
