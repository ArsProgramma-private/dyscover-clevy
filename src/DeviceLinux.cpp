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
    struct udev* udev = udev_new();
    if (!udev) {
        return false;
    }

    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        udev_unref(udev);
        return false;
    }

    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;

    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        if (dev) {
            const char* vid = udev_device_get_sysattr_value(dev, "idVendor");
            const char* pid = udev_device_get_sysattr_value(dev, "idProduct");
            if (vid && pid) {
                std::string v(vid), p(pid);
                if (IsSupported(v, p)) {
                    udev_device_unref(dev);
                    udev_enumerate_unref(enumerate);
                    udev_unref(udev);
                    return true;
                }
            }
            udev_device_unref(dev);
        }
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return false;
}
