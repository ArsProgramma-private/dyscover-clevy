// AudioControllerFallbackTest.cpp - Test audio volume control and backend fallback (T061-T062)
#include "platform/AudioController.h"
#include <iostream>

int main() {
    auto audio = CreateAudioController();
    
    // Test 1: Backend must be initialized (not Unknown)
    AudioBackendType backend = audio->backend();
    if(backend == AudioBackendType::Unknown) {
        std::cerr << "FAIL: Backend is Unknown - expected a specific backend" << std::endl;
        return 1;
    }
    
    // Test 2: Check if audio is supported on this platform
    bool isSupported = audio->supported();
    
    // Test 3: Volume operations should be available if supported
    if(isSupported) {
        // Get current volume (should be 0-65535)
        int currentVol = audio->getVolume();
        if(currentVol < 0 || currentVol > 65535) {
            std::cerr << "FAIL: getVolume() returned out-of-range value: " << currentVol << std::endl;
            return 2;
        }
        
        // Try setting volume to a test value
        bool setResult = audio->setVolume(32768); // Middle value
        if(!setResult) {
            std::cerr << "FAIL: setVolume() failed on supported platform" << std::endl;
            return 3;
        }
        
        // Verify the volume was set (allow small tolerance for backend rounding)
        int newVol = audio->getVolume();
        if(newVol < 30000 || newVol > 35000) {
            std::cerr << "FAIL: Volume not set correctly. Expected ~32768, got " << newVol << std::endl;
            return 4;
        }
        
        // Restore original volume
        audio->setVolume(currentVol);
        
        std::cout << "PASS: Audio controller works (backend: ";
        switch(backend) {
            case AudioBackendType::WindowsWaveOut: std::cout << "WindowsWaveOut"; break;
            case AudioBackendType::PulseAudio: std::cout << "PulseAudio"; break;
            case AudioBackendType::ALSA: std::cout << "ALSA"; break;
            case AudioBackendType::CoreAudio: std::cout << "CoreAudio"; break;
            default: std::cout << "Other"; break;
        }
        std::cout << ", volume: " << currentVol << ")" << std::endl;
        
    } else {
        // Platform doesn't support audio control (e.g., ChromeOS sandbox)
        if(backend != AudioBackendType::Unsupported) {
            std::cerr << "FAIL: supported()=false but backend is not Unsupported" << std::endl;
            return 5;
        }
        
        // Operations should fail gracefully
        int vol = audio->getVolume();
        if(vol != -1) {
            std::cerr << "FAIL: Expected getVolume()=-1 on unsupported platform, got " << vol << std::endl;
            return 6;
        }
        
        bool setResult = audio->setVolume(32768);
        if(setResult) {
            std::cerr << "FAIL: setVolume() should return false on unsupported platform" << std::endl;
            return 7;
        }
        
        std::cout << "PASS: Audio controller correctly reports unsupported" << std::endl;
    }
    
    return 0;
}
