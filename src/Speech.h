//
// Speech.h
//

#pragma once

#include <string>
#include <thread>

#ifndef __NO_TTS__
#include <librstts.h>
#else
// Define stub constants when TTS is disabled
#define RSTTS_VOLUME_MAX 250.0f
#endif

#include "Audio.h"
#include "Queue.h"

#ifndef __NO_TTS__
class Speech
{
public:
	Speech();
	~Speech();

	bool Init(const char* basedir, const char* lang, const char* voice);
	void Term();

	float GetSpeed();
	bool SetSpeed(float value);

	float GetVolume();
	bool SetVolume(float value);

	void Speak(std::string text);
	void Stop();

private:
	Queue<std::string> m_queue;
	std::thread m_thread;
	RSTTSInst m_rstts;
	Audio m_audio;
	bool m_quit;  // Used to signalize thread to exit

	void ThreadProc();

	static void TTSAudioCallback(RSTTSInst, const void*, size_t, void*);
};
#else
// Stubbed Speech implementation when librstts is disabled.
class Speech
{
public:
	Speech() {}
	~Speech() {}
	bool Init(const char*, const char*, const char*) { return false; }
	void Term() {}
	float GetSpeed() { return -1.0f; }
	bool SetSpeed(float) { return false; }
	float GetVolume() { return -1.0f; }
	bool SetVolume(float) { return false; }
	void Speak(std::string) {}
	void Stop() {}
};
#endif
