#if defined(__APPLE__)
// AudioControllerMac.cpp - macOS audio volume control using CoreAudio

#include "AudioController.h"
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

namespace {
    class CoreAudioController : public IAudioController {
    public:
        CoreAudioController() : m_deviceID(kAudioObjectUnknown) {
            // Get default output device
            AudioObjectPropertyAddress propertyAddress = {
                kAudioHardwarePropertyDefaultOutputDevice,
                kAudioObjectPropertyScopeGlobal,
                kAudioObjectPropertyElementMain
            };
            
            UInt32 deviceIDSize = sizeof(m_deviceID);
            OSStatus status = AudioObjectGetPropertyData(
                kAudioObjectSystemObject,
                &propertyAddress,
                0, nullptr,
                &deviceIDSize,
                &m_deviceID
            );
            
            if (status != noErr || m_deviceID == kAudioObjectUnknown) {
                m_deviceID = kAudioObjectUnknown;
            }
        }
        
        int getVolume() override {
            if (m_deviceID == kAudioObjectUnknown) return -1;
            
            AudioObjectPropertyAddress propertyAddress = {
                kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
                kAudioDevicePropertyScopeOutput,
                kAudioObjectPropertyElementMain
            };
            
            Float32 volume = 0.0f;
            UInt32 volumeSize = sizeof(volume);
            
            OSStatus status = AudioObjectGetPropertyData(
                m_deviceID,
                &propertyAddress,
                0, nullptr,
                &volumeSize,
                &volume
            );
            
            if (status != noErr) return -1;
            
            // Convert 0.0-1.0 to 0-65535
            return static_cast<int>(volume * 65535.0f);
        }
        
        bool setVolume(int value) override {
            if (m_deviceID == kAudioObjectUnknown) return false;
            if (value < 0 || value > 65535) return false;
            
            AudioObjectPropertyAddress propertyAddress = {
                kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
                kAudioDevicePropertyScopeOutput,
                kAudioObjectPropertyElementMain
            };
            
            // Convert 0-65535 to 0.0-1.0
            Float32 volume = static_cast<Float32>(value) / 65535.0f;
            
            OSStatus status = AudioObjectSetPropertyData(
                m_deviceID,
                &propertyAddress,
                0, nullptr,
                sizeof(volume),
                &volume
            );
            
            return status == noErr;
        }
        
        AudioBackendType backend() const override {
            return AudioBackendType::CoreAudio;
        }
        
        bool supported() const override {
            return m_deviceID != kAudioObjectUnknown;
        }
        
    private:
        AudioDeviceID m_deviceID;
    };
} // anonymous namespace

// Platform-specific factory function for macOS
std::unique_ptr<IAudioController> CreatePlatformAudioController() {
    return std::unique_ptr<IAudioController>(new CoreAudioController());
}

#endif // __APPLE__
