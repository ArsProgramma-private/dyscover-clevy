//
// DeviceMac.h
//

#pragma once

#include "Device.h"

class DeviceMac : public Device
{
public:
    explicit DeviceMac(IDeviceListener* pListener);

    bool IsClevyKeyboardPresent() override;
};
