//
// DeviceLinux.cpp
//

#include "DeviceLinux.h"
#include "Device.h"
#include "SupportedDevices.h"
#include <libudev.h>
#include <string>

DeviceLinux::DeviceLinux(IDeviceListener *pListener)
    : Device(pListener)
{
}

bool DeviceLinux::IsClevyKeyboardPresent()
{
    // DEPRECATED: Device detection now handled by platform abstraction layer
    // (DeviceDetectorLinux). This legacy method is retained for backward
    // compatibility but always returns false. Applications should use
    // CreateDeviceDetector() from platform/DeviceDetector.h instead.
    return false;
}
