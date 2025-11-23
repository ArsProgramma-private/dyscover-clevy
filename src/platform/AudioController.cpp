#include "AudioController.h"

namespace {
    class StubAudioController : public IAudioController {
    public:
        StubAudioController() : m_volume(32768) {}
        int getVolume() override { return m_volume; }
        bool setVolume(int v) override {
            if (v < 0 || v > 65535) return false;
            m_volume = v;
            return true;
        }
        AudioBackendType backend() const override { return AudioBackendType::PulseAudio; }
        bool supported() const override { return true; }
    private:
        int m_volume;
    };
    
}

std::unique_ptr<IAudioController> CreateAudioController() {
    return std::unique_ptr<IAudioController>(new StubAudioController());
}
