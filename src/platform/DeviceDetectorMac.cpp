#if defined(__APPLE__)
// DeviceDetectorMac.cpp - macOS device detection using IOKit
// Implements USB device enumeration and hotplug notification

#include "DeviceDetector.h"
#include "../SupportedDevices.h"
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <atomic>
#include <thread>
#include <string>
#include <iomanip>
#include <sstream>

namespace {
    constexpr PlatformCapabilities HOTPLUG_EVENTS = 1 << 0;

    // Helper: Convert IOKit numeric property to hex string (4 digits)
    std::string toHexString(uint16_t value) {
        std::ostringstream ss;
        ss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << value;
        return ss.str();
    }

    // Helper: Check if IOKit USB device matches supported VID/PID
    bool isDeviceSupported(io_service_t device) {
        // Get vendor ID
        CFTypeRef vidRef = IORegistryEntryCreateCFProperty(device, CFSTR("idVendor"), 
                                                            kCFAllocatorDefault, 0);
        if (!vidRef) return false;

        uint16_t vid = 0;
        if (CFGetTypeID(vidRef) == CFNumberGetTypeID()) {
            CFNumberGetValue((CFNumberRef)vidRef, kCFNumberSInt16Type, &vid);
        }
        CFRelease(vidRef);

        // Get product ID
        CFTypeRef pidRef = IORegistryEntryCreateCFProperty(device, CFSTR("idProduct"), 
                                                            kCFAllocatorDefault, 0);
        if (!pidRef) return false;

        uint16_t pid = 0;
        if (CFGetTypeID(pidRef) == CFNumberGetTypeID()) {
            CFNumberGetValue((CFNumberRef)pidRef, kCFNumberSInt16Type, &pid);
        }
        CFRelease(pidRef);

        // Check against SupportedDevices list
        return IsSupported(toHexString(vid), toHexString(pid));
    }

    // macOS device detector implementation using IOKit
    class MacDeviceDetector : public IDeviceDetector {
    public:
        explicit MacDeviceDetector(IDeviceDetectorListener* listener)
            : m_listener(listener),
              m_notifyPort(nullptr),
              m_runLoop(nullptr),
              m_monitoring(false)
        {
            refresh(); // Initialize presence state
        }

        ~MacDeviceDetector() override {
            stopMonitoring();
        }

        bool isPresent() const override {
            return m_present.load();
        }

        void startMonitoring() override {
            if (m_monitoring.load()) return;
            m_monitoring = true;

            // Notify listener of initial state
            if (m_listener) {
                m_listener->onDevicePresenceChanged(m_present.load());
            }

            // Create notification port and run loop for hotplug events
            m_notifyPort = IONotificationPortCreate(kIOMainPortDefault);
            if (!m_notifyPort) return;

            // Set up matching dictionary for USB devices
            CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
            if (!matchingDict) {
                IONotificationPortDestroy(m_notifyPort);
                m_notifyPort = nullptr;
                return;
            }

            // Retain dictionary for both add and remove notifications
            matchingDict = (CFMutableDictionaryRef)CFRetain(matchingDict);

            // Register for device arrival notifications
            io_iterator_t addedIter;
            kern_return_t kr = IOServiceAddMatchingNotification(
                m_notifyPort, kIOMatchedNotification, matchingDict,
                deviceAddedCallback, this, &addedIter);

            if (kr == KERN_SUCCESS) {
                // Process existing matches to arm the notification
                io_service_t device;
                while ((device = IOIteratorNext(addedIter))) {
                    IOObjectRelease(device);
                }
            }

            // Register for device removal notifications
            matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
            io_iterator_t removedIter;
            kr = IOServiceAddMatchingNotification(
                m_notifyPort, kIOTerminatedNotification, matchingDict,
                deviceRemovedCallback, this, &removedIter);

            if (kr == KERN_SUCCESS) {
                // Process existing matches to arm the notification
                io_service_t device;
                while ((device = IOIteratorNext(removedIter))) {
                    IOObjectRelease(device);
                }
            }

            // Start monitoring thread with run loop
            m_monitorThread = std::thread([this]() {
                m_runLoop = CFRunLoopGetCurrent();
                CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(m_notifyPort);
                CFRunLoopAddSource(m_runLoop, runLoopSource, kCFRunLoopDefaultMode);
                
                // Run loop until stopped
                while (m_monitoring.load()) {
                    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.5, false);
                }
                
                CFRunLoopRemoveSource(m_runLoop, runLoopSource, kCFRunLoopDefaultMode);
            });
        }

        void stopMonitoring() override {
            if (!m_monitoring.load()) return;
            m_monitoring = false;

            // Stop run loop
            if (m_runLoop) {
                CFRunLoopStop(m_runLoop);
                m_runLoop = nullptr;
            }

            // Wait for monitoring thread
            if (m_monitorThread.joinable()) {
                m_monitorThread.join();
            }

            // Clean up notification port
            if (m_notifyPort) {
                IONotificationPortDestroy(m_notifyPort);
                m_notifyPort = nullptr;
            }
        }

        void refresh() override {
            // Enumerate USB devices using IOKit
            CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
            if (!matchingDict) {
                m_present = false;
                return;
            }

            io_iterator_t iter;
            kern_return_t kr = IOServiceGetMatchingServices(kIOMainPortDefault, matchingDict, &iter);
            if (kr != KERN_SUCCESS) {
                m_present = false;
                return;
            }

            bool found = false;
            io_service_t device;
            while ((device = IOIteratorNext(iter))) {
                if (isDeviceSupported(device)) {
                    found = true;
                    IOObjectRelease(device);
                    break;
                }
                IOObjectRelease(device);
            }
            IOObjectRelease(iter);

            bool oldPresence = m_present.exchange(found);
            
            // Notify listener if state changed and monitoring is active
            if (m_monitoring.load() && m_listener && (oldPresence != found)) {
                m_listener->onDevicePresenceChanged(found);
            }
        }

        PlatformCapabilities capabilities() const override {
            return HOTPLUG_EVENTS; // macOS supports native hotplug via IOKit notifications
        }

    private:
        // Static callback for device added notifications
        static void deviceAddedCallback(void* refCon, io_iterator_t iterator) {
            MacDeviceDetector* detector = static_cast<MacDeviceDetector*>(refCon);
            
            // Process all devices in iterator
            io_service_t device;
            while ((device = IOIteratorNext(iterator))) {
                if (isDeviceSupported(device)) {
                    detector->refresh(); // Re-scan and notify if changed
                }
                IOObjectRelease(device);
            }
        }

        // Static callback for device removed notifications
        static void deviceRemovedCallback(void* refCon, io_iterator_t iterator) {
            MacDeviceDetector* detector = static_cast<MacDeviceDetector*>(refCon);
            
            // Process all devices in iterator
            io_service_t device;
            while ((device = IOIteratorNext(iterator))) {
                detector->refresh(); // Re-scan and notify if changed
                IOObjectRelease(device);
            }
        }

        IDeviceDetectorListener* m_listener;
        IONotificationPortRef m_notifyPort;
        CFRunLoopRef m_runLoop;
        std::atomic<bool> m_present{false};
        std::atomic<bool> m_monitoring{false};
        std::thread m_monitorThread;
    };

} // anonymous namespace

// Platform-specific factory function for macOS
std::unique_ptr<IDeviceDetector> CreatePlatformDeviceDetector(IDeviceDetectorListener* listener) {
    return std::unique_ptr<IDeviceDetector>(new MacDeviceDetector(listener));
}

#endif // __APPLE__
