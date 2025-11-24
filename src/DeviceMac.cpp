//
// DeviceMac.cpp
//

#include "DeviceMac.h"

DeviceMac::DeviceMac(IDeviceListener* pListener)
    : Device(pListener)
{
}

bool DeviceMac::IsClevyKeyboardPresent()
{
    // Device detection runs through DeviceDetectorMac; keep legacy API stubbed.
    return false;
}
