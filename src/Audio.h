//
// Audio.h
//

#pragma once

#ifndef __NO_PORTAUDIO__
#include <portaudio.h>

class Audio
{
public:
	Audio();
	~Audio();

	bool Open(int channels, int samplerate, PaSampleFormat sampleformat);
	void Close();

	bool Write(const void* audiodata, unsigned long audiodatalen);
	void Stop();

private:
	PaStream* m_pStream;
};
#else
// Stubbed Audio implementation when PortAudio is disabled.
class Audio
{
public:
	Audio() {}
	~Audio() {}

	bool Open(int, int, int) { return false; }
	void Close() {}
	bool Write(const void*, unsigned long) { return false; }
	void Stop() {}
};
#endif
