//
// Device.cpp
//

#include <wx/log.h>

#include "Device.h"

#ifdef __PLATFORM_WINDOWS__
#include "DeviceWindows.h"
#elif defined(__PLATFORM_MAC__)
#include "DeviceMac.h"
#elif defined(__PLATFORM_LINUX__)
#include "DeviceLinux.h"
#endif

// Removed runtime JSON configuration; device list is now compile-time.

Device* Device::Create(IDeviceListener* pListener)
{
#ifdef __PLATFORM_WINDOWS__
    return new DeviceWindows(pListener);
#elif defined(__PLATFORM_MAC__)
    return new DeviceMac(pListener);
#elif defined(__PLATFORM_LINUX__)
    return new DeviceLinux(pListener);
#else
    (void)pListener;
    return nullptr;
#endif
}

Device::Device(IDeviceListener* pListener)
{
    m_pListener = pListener;

    m_bIsClevyKeyboardPresent = false;
}

Device::~Device()
{
}

void Device::InitClevyKeyboardPresence()
{
    m_bIsClevyKeyboardPresent = IsClevyKeyboardPresent();

    wxLogDebug("Device::InitClevyKeyboardPresence()  presence = %d", m_bIsClevyKeyboardPresent);
}

void Device::RefreshClevyKeyboardPresence()
{
    // TODO: Move this logic to Core.
    bool bIsClevyKeyboardPresent = IsClevyKeyboardPresent();
    wxLogDebug("Device::RefreshClevyKeyboardPresence()  presence = %d -> %d", m_bIsClevyKeyboardPresent, bIsClevyKeyboardPresent);
    if (bIsClevyKeyboardPresent && !m_bIsClevyKeyboardPresent)
    {
        m_pListener->OnClevyKeyboardConnected();
    }
    else if (!bIsClevyKeyboardPresent && m_bIsClevyKeyboardPresent)
    {
        m_pListener->OnClevyKeyboardDisconnected();
    }
    m_bIsClevyKeyboardPresent = bIsClevyKeyboardPresent;
}
