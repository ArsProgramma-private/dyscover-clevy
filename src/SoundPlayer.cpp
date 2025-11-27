//
// SoundPlayer.cpp
//

#include <wx/filename.h>

#include "ResourceLoader.h"
#include "SoundPlayer.h"
#include "DebugLogger.h"

#ifndef __NO_PORTAUDIO__

#include <fstream>
#include <cstring>
#include <algorithm>
#include <cmath>

// WAV file header structures
#pragma pack(push, 1)
struct WavHeader {
    char riff[4];           // "RIFF"
    uint32_t fileSize;
    char wave[4];           // "WAVE"
};

struct WavFormat {
    char fmt[4];            // "fmt "
    uint32_t fmtSize;
    uint16_t audioFormat;   // 1 = PCM
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

struct WavData {
    char data[4];           // "data"
    uint32_t dataSize;
};
#pragma pack(pop)

SoundPlayer::SoundPlayer() 
    : m_stream(nullptr), m_initialized(false)
{
    m_soundFilesPath = GetSoundFilesPath();
    
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return;
    }
    
    // Open audio stream with stereo output at 44.1kHz
    err = Pa_OpenDefaultStream(&m_stream,
                               0,          // no input channels
                               2,          // stereo output
                               paFloat32,  // 32-bit float output
                               44100,      // sample rate
                               256,        // frames per buffer
                               AudioCallback,
                               this);      // user data
    
    if (err != paNoError) {
        Pa_Terminate();
        return;
    }
    
    err = Pa_StartStream(m_stream);
    if (err != paNoError) {
        Pa_CloseStream(m_stream);
        Pa_Terminate();
        return;
    }
    
    m_initialized = true;
}

SoundPlayer::~SoundPlayer()
{
    if (m_initialized) {
        if (m_stream) {
            Pa_StopStream(m_stream);
            Pa_CloseStream(m_stream);
        }
        Pa_Terminate();
    }
}

std::shared_ptr<SoundData> SoundPlayer::LoadWavFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }
    
    // Read WAV header
    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));
    
    if (std::strncmp(header.riff, "RIFF", 4) != 0 || 
        std::strncmp(header.wave, "WAVE", 4) != 0) {
        return nullptr;
    }
    
    // Read format chunk
    WavFormat format;
    file.read(reinterpret_cast<char*>(&format), sizeof(WavFormat));
    
    if (std::strncmp(format.fmt, "fmt ", 4) != 0 || format.audioFormat != 1) {
        return nullptr;
    }
    
    // Read data chunk
    WavData dataHeader;
    file.read(reinterpret_cast<char*>(&dataHeader), sizeof(WavData));
    
    if (std::strncmp(dataHeader.data, "data", 4) != 0) {
        return nullptr;
    }
    
    // Allocate buffer for audio data
    size_t numSamples = dataHeader.dataSize / (format.bitsPerSample / 8);
    std::vector<float> samples(numSamples);
    
    // Read and convert samples to float
    if (format.bitsPerSample == 16) {
        std::vector<int16_t> rawSamples(numSamples);
        file.read(reinterpret_cast<char*>(rawSamples.data()), dataHeader.dataSize);
        
        for (size_t i = 0; i < numSamples; ++i) {
            samples[i] = rawSamples[i] / 32768.0f;
        }
    } else if (format.bitsPerSample == 8) {
        std::vector<uint8_t> rawSamples(numSamples);
        file.read(reinterpret_cast<char*>(rawSamples.data()), dataHeader.dataSize);
        
        for (size_t i = 0; i < numSamples; ++i) {
            samples[i] = (rawSamples[i] - 128) / 128.0f;
        }
    } else {
        return nullptr;
    }
    
    auto soundData = std::make_shared<SoundData>();
    soundData->samples = std::move(samples);
    soundData->channels = format.numChannels;
    soundData->sampleRate = format.sampleRate;
    
    return soundData;
}

void SoundPlayer::PlaySoundFile(const std::string& soundfile)
{
    if (!m_initialized) {
        return;
    }
    
    wxFileName filename(m_soundFilesPath, soundfile);
    auto soundData = LoadWavFile(filename.GetFullPath().ToStdString());
    
    if (!soundData) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Fade out all currently playing sounds
    for (auto& sound : m_playingSounds) {
        sound.fadingOut = true;
    }
    
    // Add new sound with fade in
    m_playingSounds.emplace_back(soundData);
}

void SoundPlayer::StopPlaying()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Fade out all playing sounds
    for (auto& sound : m_playingSounds) {
        sound.fadingOut = true;
    }
}

int SoundPlayer::AudioCallback(const void* inputBuffer, void* outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void* userData)
{
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;
    
    SoundPlayer* player = static_cast<SoundPlayer*>(userData);
    float* out = static_cast<float*>(outputBuffer);
    
    player->MixAudio(out, framesPerBuffer);
    
    return paContinue;
}

void SoundPlayer::MixAudio(float* outputBuffer, unsigned long framesPerBuffer)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clear output buffer
    std::memset(outputBuffer, 0, framesPerBuffer * 2 * sizeof(float));
    
    // Mix all playing sounds
    auto it = m_playingSounds.begin();
    while (it != m_playingSounds.end()) {
        PlayingSound& sound = *it;
        bool finished = false;
        
        // Calculate fade rate (volume change per sample)
        float fadeRate = 1.0f / (FADE_DURATION * 44100.0f);
        
        for (unsigned long frame = 0; frame < framesPerBuffer; ++frame) {
            // Update volume based on fade state
            if (sound.fadingOut) {
                sound.volume = std::max(0.0f, sound.volume - fadeRate);
                if (sound.volume <= 0.0f) {
                    finished = true;
                    break;
                }
            } else {
                sound.volume = std::min(1.0f, sound.volume + fadeRate);
            }
            
            // Check if we've reached the end of the sound
            size_t samplesPerFrame = sound.data->channels;
            if (sound.playPosition >= sound.data->samples.size()) {
                finished = true;
                break;
            }
            
            // Mix the samples into the output
            for (int ch = 0; ch < 2; ++ch) {
                float sample = 0.0f;
                
                if (sound.data->channels == 1) {
                    // Mono: use same sample for both channels
                    sample = sound.data->samples[sound.playPosition];
                } else if (sound.data->channels == 2) {
                    // Stereo: use appropriate channel
                    size_t sampleIndex = sound.playPosition + ch;
                    if (sampleIndex < sound.data->samples.size()) {
                        sample = sound.data->samples[sampleIndex];
                    }
                }
                
                outputBuffer[frame * 2 + ch] += sample * sound.volume;
            }
            
            sound.playPosition += samplesPerFrame;
        }
        
        if (finished) {
            it = m_playingSounds.erase(it);
        } else {
            ++it;
        }
    }
    
    // Apply soft clipping to prevent harsh distortion
    for (unsigned long i = 0; i < framesPerBuffer * 2; ++i) {
        if (outputBuffer[i] > 1.0f) {
            outputBuffer[i] = 1.0f;
        } else if (outputBuffer[i] < -1.0f) {
            outputBuffer[i] = -1.0f;
        }
    }
}

#else
// Fallback implementation when PortAudio is disabled
#include <wx/sound.h>

SoundPlayer::SoundPlayer()
{
    m_soundFilesPath = GetSoundFilesPath();
    wxLogDebug("SoundPlayer initialized with path: %s", m_soundFilesPath);
}

SoundPlayer::~SoundPlayer()
{
}

void SoundPlayer::PlaySoundFile(const std::string& soundfile)
{
    wxFileName filename(m_soundFilesPath, soundfile);
    wxSound::Play(filename.GetFullPath(), wxSOUND_ASYNC);
}

void SoundPlayer::StopPlaying()
{
#ifdef _WIN32
    PlaySoundA(NULL, NULL, 0); // Stop any playing sound
#else
    wxSound::Stop();
#endif
}
#endif // __NO_PORTAUDIO__
