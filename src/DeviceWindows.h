//
// DeviceWindows.h
//

#pragma once

#include <wx/frame.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cfgmgr32.h>

#include "Device.h"

class DeviceWindows : public Device, public wxFrame
{
public:
    DeviceWindows(IDeviceListener* pListener);
    virtual ~DeviceWindows();

    virtual bool IsClevyKeyboardPresent() override;

private:
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;

    bool DoesContainClevyKeyboard(DEVINST hDevice);
    bool extractVidPid(const TCHAR* hardwareId, std::string& vid, std::string& pid);
};
