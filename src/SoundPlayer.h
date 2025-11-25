//
// SoundPlayer.h
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#ifndef __NO_PORTAUDIO__
#include <portaudio.h>

// Structure to hold loaded sound data
struct SoundData {
    std::vector<float> samples;
    int channels;
    int sampleRate;
};

// Structure to track a playing sound with fade state
struct PlayingSound {
    std::shared_ptr<SoundData> data;
    size_t playPosition;
    float volume;
    bool fadingOut;
    
    PlayingSound(std::shared_ptr<SoundData> d) 
        : data(d), playPosition(0), volume(0.0f), fadingOut(false) {}
};

class SoundPlayer
{
public:
	SoundPlayer();
	~SoundPlayer();

	void PlaySoundFile(const std::string& filename);
	void StopPlaying();

private:
	std::string m_soundFilesPath;
    PaStream* m_stream;
    std::vector<PlayingSound> m_playingSounds;
    std::mutex m_mutex;
    bool m_initialized;
    
    // Crossfade duration in seconds
    static constexpr float FADE_DURATION = 0.15f;
    
    std::shared_ptr<SoundData> LoadWavFile(const std::string& filepath);
    static int AudioCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData);
    void MixAudio(float* outputBuffer, unsigned long framesPerBuffer);
};

#else
// Fallback implementation when PortAudio is disabled
class SoundPlayer
{
public:
	SoundPlayer();
	~SoundPlayer();

	void PlaySoundFile(const std::string& filename);
	void StopPlaying();

private:
	std::string m_soundFilesPath;
};
#endif
