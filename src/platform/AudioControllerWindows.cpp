#if defined(_WIN32)
// AudioControllerWindows.cpp - Windows audio volume control using waveOut API
// Refactored from AudioLevel.cpp to support platform abstraction layer

#include "AudioController.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>

namespace {
    class WindowsAudioController : public IAudioController {
    public:
        WindowsAudioController() = default;
        
        int getVolume() override {
            DWORD dwVolume;
            MMRESULT result = waveOutGetVolume(0, &dwVolume);
            if (result != MMSYSERR_NOERROR) {
                // Return -1 on error (e.g., no audio device)
                return -1;
            }
            
            // waveOut returns volume as DWORD with left/right channels
            // Extract left channel (low word) as primary volume
            return dwVolume & 0xFFFF;
        }
        
        bool setVolume(int value) override {
            // Validate range
            if (value < 0 || value > 65535) {
                return false;
            }
            
            // Set both left and right channels to the same value
            DWORD dwVolume = MAKELONG(value, value);
            MMRESULT result = waveOutSetVolume(0, dwVolume);
            
            return result == MMSYSERR_NOERROR;
        }
        
        AudioBackendType backend() const override {
            return AudioBackendType::WindowsWaveOut;
        }
        
        bool supported() const override {
            // Check if we can query volume (indicates audio device present)
            DWORD dwVolume;
            MMRESULT result = waveOutGetVolume(0, &dwVolume);
            return result == MMSYSERR_NOERROR;
        }
    };
} // anonymous namespace

// Platform-specific factory function for Windows
std::unique_ptr<IAudioController> CreatePlatformAudioController() {
    return std::unique_ptr<IAudioController>(new WindowsAudioController());
}

#endif // _WIN32
