#if defined(__linux__) && !defined(__ANDROID__)
// AudioControllerLinux.cpp - Linux audio volume control
// Primary: libpulse (PulseAudio/PipeWire compatibility)
// Fallback: ALSA mixer API

#include "AudioController.h"
#include <cstring>
#include <memory>

// Try PulseAudio first (most common on modern Linux)
#ifdef HAVE_LIBPULSE
#include <pulse/pulseaudio.h>

namespace {
    class PulseAudioController : public IAudioController {
    public:
        PulseAudioController() : m_volume(-1), m_initialized(false) {
            // Initialize PulseAudio connection
            // This is a simplified synchronous approach
            // A full implementation would use async API with mainloop
        }
        
        int getVolume() override {
            // TODO: Implement PA context and sink volume query
            // For now, return cached value or error
            return m_volume;
        }
        
        bool setVolume(int value) override {
            if (value < 0 || value > 65535) return false;
            
            // TODO: Implement PA context and sink volume set
            // For now, cache the value
            m_volume = value;
            return true;
        }
        
        AudioBackendType backend() const override {
            return AudioBackendType::PulseAudio;
        }
        
        bool supported() const override {
            return m_initialized;
        }
        
    private:
        int m_volume;
        bool m_initialized;
    };
}
#endif // HAVE_LIBPULSE

// ALSA fallback implementation
#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>

namespace {
    class ALSAAudioController : public IAudioController {
    public:
        ALSAAudioController() {
            // Try to open ALSA mixer for "Master" control
            int err = snd_mixer_open(&m_mixer, 0);
            if (err < 0) {
                m_mixer = nullptr;
                return;
            }
            
            err = snd_mixer_attach(m_mixer, "default");
            if (err < 0) {
                snd_mixer_close(m_mixer);
                m_mixer = nullptr;
                return;
            }
            
            err = snd_mixer_selem_register(m_mixer, nullptr, nullptr);
            if (err < 0) {
                snd_mixer_close(m_mixer);
                m_mixer = nullptr;
                return;
            }
            
            err = snd_mixer_load(m_mixer);
            if (err < 0) {
                snd_mixer_close(m_mixer);
                m_mixer = nullptr;
                return;
            }
            
            // Find Master element
            snd_mixer_selem_id_t *sid;
            snd_mixer_selem_id_alloca(&sid);
            snd_mixer_selem_id_set_index(sid, 0);
            snd_mixer_selem_id_set_name(sid, "Master");
            
            m_elem = snd_mixer_find_selem(m_mixer, sid);
            if (!m_elem) {
                snd_mixer_close(m_mixer);
                m_mixer = nullptr;
            }
        }
        
        ~ALSAAudioController() {
            if (m_mixer) {
                snd_mixer_close(m_mixer);
            }
        }
        
        int getVolume() override {
            if (!m_mixer || !m_elem) return -1;
            
            long min, max, volume;
            snd_mixer_selem_get_playback_volume_range(m_elem, &min, &max);
            snd_mixer_selem_get_playback_volume(m_elem, SND_MIXER_SCHN_MONO, &volume);
            
            // Convert ALSA range to 0-65535
            if (max > min) {
                return static_cast<int>((volume - min) * 65535 / (max - min));
            }
            return 0;
        }
        
        bool setVolume(int value) override {
            if (!m_mixer || !m_elem) return false;
            if (value < 0 || value > 65535) return false;
            
            long min, max;
            snd_mixer_selem_get_playback_volume_range(m_elem, &min, &max);
            
            // Convert 0-65535 to ALSA range
            long alsaVolume = min + (value * (max - min) / 65535);
            
            int err = snd_mixer_selem_set_playback_volume_all(m_elem, alsaVolume);
            return err >= 0;
        }
        
        AudioBackendType backend() const override {
            return AudioBackendType::ALSA;
        }
        
        bool supported() const override {
            return m_mixer != nullptr && m_elem != nullptr;
        }
        
    private:
        snd_mixer_t *m_mixer = nullptr;
        snd_mixer_elem_t *m_elem = nullptr;
    };
}
#endif // HAVE_ALSA

// Stub for when no audio library available
namespace {
    class UnsupportedAudioController : public IAudioController {
    public:
        int getVolume() override { return -1; }
        bool setVolume(int) override { return false; }
        AudioBackendType backend() const override { return AudioBackendType::Unsupported; }
        bool supported() const override { return false; }
    };
}

// Platform-specific factory function for Linux
std::unique_ptr<IAudioController> CreatePlatformAudioController() {
#ifdef HAVE_LIBPULSE
    // Try PulseAudio first (works with PipeWire too)
    auto pa = std::unique_ptr<IAudioController>(new PulseAudioController());
    if (pa->supported()) {
        return pa;
    }
#endif

#ifdef HAVE_ALSA
    // Fall back to ALSA
    auto alsa = std::unique_ptr<IAudioController>(new ALSAAudioController());
    if (alsa->supported()) {
        return alsa;
    }
#endif

    // No audio backend available
    return std::unique_ptr<IAudioController>(new UnsupportedAudioController());
}

#endif // __linux__ && !__ANDROID__
