#if defined(_WIN32)
// DeviceDetectorWindows.cpp - platform-specific device enumeration using SetupAPI / CM_* calls

#include "DeviceDetector.h"
#include "SupportedDevices.h"
#include "platform/PlatformUtils.h"
#include <windows.h>
#include <cfgmgr32.h>
#include <string>

class DeviceDetectorWindows : public IDeviceDetector {
public:
    explicit DeviceDetectorWindows(IDeviceDetectorListener* listener) : m_listener(listener) {}

    bool isPresent() const override {
        DEVINST hRootDevice;
        CONFIGRET cr = CM_Locate_DevNode(&hRootDevice, nullptr, CM_LOCATE_DEVINST_NORMAL);
        if (cr != CR_SUCCESS) return false;
        return containsClevyKeyboard(hRootDevice);
    }

    void startMonitoring() override {
        // On Windows we can immediately notify listener with current presence
        if (m_listener) m_listener->onDevicePresenceChanged(isPresent());
        // Full hotplug event handling will be wired in a later step (hook/window message)
    }

    void stopMonitoring() override { /* noop */ }
    void refresh() override { if (m_listener) m_listener->onDevicePresenceChanged(isPresent()); }
    PlatformCapabilities capabilities() const override { return 1 << 0; /* HOTPLUG_EVENTS */ }

private:
    bool containsClevyKeyboard(DEVINST hDevice) const {
        while (true) {
            TCHAR szHardwareId[1024] = {0};
            ULONG ulHardwareId = sizeof(szHardwareId);
            CONFIGRET cr = CM_Get_DevNode_Registry_Property(hDevice, CM_DRP_HARDWAREID, nullptr, szHardwareId, &ulHardwareId, 0);
            if (cr == CR_SUCCESS) {
                // Convert TCHAR* to std::string
#ifdef UNICODE
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, szHardwareId, -1, nullptr, 0, nullptr, nullptr);
                if (size_needed > 0) {
                    std::string hid(size_needed - 1, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, szHardwareId, -1, &hid[0], size_needed, nullptr, nullptr);
                    std::string vid, pid;
                    if (PlatformUtils::extractVidPid(hid, vid, pid) && IsSupported(vid, pid)) return true;
                }
#else
                std::string hid(szHardwareId);
                std::string vid, pid;
                if (PlatformUtils::extractVidPid(hid, vid, pid) && IsSupported(vid, pid)) return true;
#endif
            }

            DEVINST hChild;
            cr = CM_Get_Child(&hChild, hDevice, 0);
            if (cr == CR_SUCCESS) {
                if (containsClevyKeyboard(hChild)) return true;
            }

            DEVINST hNext;
            cr = CM_Get_Sibling(&hNext, hDevice, 0);
            if (cr != CR_SUCCESS) break;
            hDevice = hNext;
        }
        return false;
    }

    IDeviceDetectorListener* m_listener{nullptr};
};

#endif // _WIN32

#if defined(_WIN32)
std::unique_ptr<IDeviceDetector> CreatePlatformDeviceDetector(IDeviceDetectorListener* listener) {
    return std::unique_ptr<IDeviceDetector>(new DeviceDetectorWindows(listener));
}
#endif
