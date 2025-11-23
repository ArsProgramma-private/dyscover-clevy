#pragma once
#include <memory>

enum class AudioBackendType { Unknown, WindowsWaveOut, PulseAudio, ALSA, CoreAudio, Unsupported };

class IAudioController {
public:
    virtual int getVolume() = 0; // 0-65535, -1 on error
    virtual bool setVolume(int value) = 0; // false if unsupported
    virtual AudioBackendType backend() const = 0;
    virtual bool supported() const = 0;
    virtual ~IAudioController() = default;
};

std::unique_ptr<IAudioController> CreateAudioController();
