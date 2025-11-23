#if defined(_WIN32)
// DeviceDetectorWindows.cpp - Windows device detection using Config Manager API
// Refactored from DeviceWindows.cpp to support platform abstraction layer

#include "DeviceDetector.h"
#include "../SupportedDevices.h"
#include <windows.h>
#include <cfgmgr32.h>
#include <wx/log.h>
#include <wx/frame.h>
#include <atomic>
#include <string>
#include <algorithm>

namespace {
    constexpr PlatformCapabilities HOTPLUG_EVENTS = 1 << 0;

    // Helper: Extract VID/PID from Windows hardware ID string (USB\VID_xxxx&PID_yyyy)
    bool extractVidPid(const TCHAR* hardwareId, std::string& vid, std::string& pid) {
        // Convert TCHAR* to std::string
#ifdef UNICODE
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, hardwareId, -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0) return false;
        std::string hid(size_needed - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, hardwareId, -1, &hid[0], size_needed, nullptr, nullptr);
#else
        std::string hid = hardwareId;
#endif

        // Convert to uppercase for consistent parsing
        std::transform(hid.begin(), hid.end(), hid.begin(), 
                      [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

        // Parse VID_xxxx and PID_xxxx
        size_t vidPos = hid.find("VID_");
        if (vidPos != std::string::npos) {
            vid = hid.substr(vidPos + 4, 4);
            size_t pidPos = hid.find("PID_", vidPos);
            if (pidPos != std::string::npos) {
                pid = hid.substr(pidPos + 4, 4);
                return true;
            }
        }
        return false;
    }

    // Helper: Recursively search device tree for supported USB device
    bool DoesContainSupportedDevice(DEVINST hDevice) {
        while (true) {
            // Retrieve hardware ID from device node
            TCHAR szHardwareId[1024] = { 0 };
            ULONG ulHardwareId = 1024;
            CONFIGRET cr = CM_Get_DevNode_Registry_Property(
                hDevice, CM_DRP_HARDWAREID, nullptr, szHardwareId, &ulHardwareId, 0);
            
            if (cr == CR_SUCCESS) {
                std::string vid, pid;
                if (extractVidPid(szHardwareId, vid, pid)) {
                    // Check against SupportedDevices.h list
                    if (IsSupported(vid, pid)) {
                        return true;
                    }
                }

                // Legacy: Keep Bluetooth hardware ID check (BTHENUM\DEV_01000141)
                // This supports older Clevy Bluetooth keyboards
                TCHAR szUpperHwId[1024];
#ifdef UNICODE
                wcscpy_s(szUpperHwId, szHardwareId);
                _wcsupr_s(szUpperHwId, _countof(szUpperHwId));
#else
                strcpy_s(szUpperHwId, szHardwareId);
                _strupr_s(szUpperHwId, _countof(szUpperHwId));
#endif
                if (_tcsstr(szUpperHwId, TEXT("BTHENUM\\DEV_01000141")) != nullptr) {
                    return true;
                }
            }

            // Recursively search child devices
            DEVINST hChildDevice;
            cr = CM_Get_Child(&hChildDevice, hDevice, 0);
            if (cr == CR_SUCCESS) {
                if (DoesContainSupportedDevice(hChildDevice)) {
                    return true;
                }
            }

            // Move to next sibling
            DEVINST hNextDevice;
            cr = CM_Get_Sibling(&hNextDevice, hDevice, 0);
            if (cr != CR_SUCCESS) break;
            hDevice = hNextDevice;
        }

        return false;
    }

    // Windows device detector implementation using wxFrame to receive WM_DEVICECHANGE messages
    class WindowsDeviceDetector : public IDeviceDetector, public wxFrame {
    public:
        explicit WindowsDeviceDetector(IDeviceDetectorListener* listener)
            : wxFrame(nullptr, wxID_ANY, wxEmptyString),
              m_listener(listener),
              m_monitoring(false)
        {
            refresh(); // Initialize presence state on creation
        }

        ~WindowsDeviceDetector() override {
            stopMonitoring();
        }

        bool isPresent() const override {
            return m_present.load();
        }

        void startMonitoring() override {
            if (m_monitoring.load()) return;
            m_monitoring = true;

            // Notify listener of initial device presence state
            if (m_listener) {
                m_listener->onDevicePresenceChanged(m_present.load());
            }

            // wxFrame automatically receives WM_DEVICECHANGE broadcast messages
            // No explicit registration needed for device notifications
        }

        void stopMonitoring() override {
            m_monitoring = false;
        }

        void refresh() override {
            // Enumerate device tree starting from root to check for supported device
            DEVINST hRootDevice;
            CONFIGRET cr = CM_Locate_DevNode(&hRootDevice, nullptr, CM_LOCATE_DEVINST_NORMAL);
            
            bool newPresence = false;
            if (cr == CR_SUCCESS) {
                newPresence = DoesContainSupportedDevice(hRootDevice);
            } else {
                wxLogWarning("DeviceDetectorWindows: Failed to locate root device node (CR=%d)", cr);
            }

            bool oldPresence = m_present.exchange(newPresence);
            
            // Notify listener if state changed and monitoring is active
            if (m_monitoring.load() && m_listener && (oldPresence != newPresence)) {
                m_listener->onDevicePresenceChanged(newPresence);
            }
        }

        PlatformCapabilities capabilities() const override {
            return HOTPLUG_EVENTS; // Windows supports native hotplug via WM_DEVICECHANGE
        }

    protected:
        // Override wxFrame message handler to intercept WM_DEVICECHANGE
        WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override {
            if (message == WM_DEVICECHANGE && m_monitoring.load()) {
                // Device change event detected - refresh device presence state
                refresh();
            }
            return wxFrame::MSWWindowProc(message, wParam, lParam);
        }

    private:
        IDeviceDetectorListener* m_listener;
        std::atomic<bool> m_present{false};
        std::atomic<bool> m_monitoring{false};
    };

} // anonymous namespace

// Platform-specific factory function for Windows
std::unique_ptr<IDeviceDetector> CreatePlatformDeviceDetector(IDeviceDetectorListener* listener) {
    return std::unique_ptr<IDeviceDetector>(new WindowsDeviceDetector(listener));
}

#endif // _WIN32
