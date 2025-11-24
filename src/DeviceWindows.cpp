//
// DeviceWindows.cpp
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wx/log.h>

#include "DeviceWindows.h"
#include "Device.h"
#include "SupportedDevices.h"
#include <string>

DeviceWindows::DeviceWindows(IDeviceListener* pListener)
    : Device(pListener), wxFrame(nullptr, wxID_ANY, wxEmptyString)
{
    InitClevyKeyboardPresence();
}

DeviceWindows::~DeviceWindows()
{
}

WXLRESULT DeviceWindows::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    (void)wParam;
    (void)lParam;

    if (message == WM_DEVICECHANGE)
    {
        RefreshClevyKeyboardPresence();
    }

    return 0;
}

bool DeviceWindows::IsClevyKeyboardPresent()
{
    // DEPRECATED: Device detection now handled by platform abstraction layer
    // (DeviceDetectorWindows). This legacy method is retained for backward
    // compatibility but always returns false. Applications should use
    // CreateDeviceDetector() from platform/DeviceDetector.h instead.
    return false;
}

bool DeviceWindows::DoesContainClevyKeyboard(DEVINST hDevice)
{
    while (true)
    {
        // Retrieve hardware ID
        TCHAR szHardwareId[1024] = { 0 };
        ULONG ulHardwareId = 1024;
        CONFIGRET cr = CM_Get_DevNode_Registry_Property(hDevice, CM_DRP_HARDWAREID, nullptr, szHardwareId, &ulHardwareId, 0);
        if (cr == CR_SUCCESS)
        {
            // Convert to upper case (safe replacement for deprecated _tcsupr)
            CharUpperBuff(szHardwareId, static_cast<DWORD>(_tcslen(szHardwareId)));

            // Check if it matches known Clevy Keyboard hardware IDs
            std::string vid, pid;
            if (this->extractVidPid(szHardwareId, vid, pid)) {
                if (IsSupported(vid, pid)) {
                    return true;
                }
            }

            // Keep BT check for now
            if (_tcsstr(szHardwareId, TEXT("BTHENUM\\DEV_01000141")) != nullptr) {
                return true;
            }
        }

        // Recursively iterate through child devices
        DEVINST hChildDevice;
        cr = CM_Get_Child(&hChildDevice, hDevice, 0);
        if (cr == CR_SUCCESS)
        {
            bool contains = DoesContainClevyKeyboard(hChildDevice);
            if (contains)
            {
                return true;
            }
        }

        // Move to next sibling device
        DEVINST hNextDevice;
        cr = CM_Get_Sibling(&hNextDevice, hDevice, 0);
        if (cr != CR_SUCCESS)
        {
            break;
        }
        hDevice = hNextDevice;
    }

    return false;
}

bool DeviceWindows::extractVidPid(const TCHAR* hardwareId, std::string& vid, std::string& pid)
{
    // Convert TCHAR* to std::string properly
    #ifdef UNICODE
        // Convert wide string to narrow string
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, hardwareId, -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0) {
            return false;
        }
        std::string hid(size_needed - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, hardwareId, -1, &hid[0], size_needed, nullptr, nullptr);
    #else
        std::string hid = hardwareId;
    #endif
    
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
