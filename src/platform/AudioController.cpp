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
        AudioBackendType backend() const override { return AudioBackendType::Unknown; }
        bool supported() const override { return false; }
    private:
        int m_volume;
    };
}

std::unique_ptr<IAudioController> CreateAudioController() {
#if defined(__USE_PLATFORM_AUDIO__)
    // Platform-specific implementations provide CreatePlatformAudioController
    extern std::unique_ptr<IAudioController> CreatePlatformAudioController();
    try {
        return CreatePlatformAudioController();
    } catch (...) {
        // Fall back to stub on error
        return std::unique_ptr<IAudioController>(new StubAudioController());
    }
#else
    // Default to stub implementation for tests
    return std::unique_ptr<IAudioController>(new StubAudioController());
#endif
}
