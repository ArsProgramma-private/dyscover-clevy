//
// AudioLevel.cpp
//

#include "AudioLevel.h"
#include "platform/AudioController.h"
#include <memory>

namespace {
	// Singleton accessor for the audio controller
	IAudioController* GetController() {
		static std::unique_ptr<IAudioController> s_controller = CreateAudioController();
		return s_controller.get();
	}
}

int GetAudioVolume()
{
	IAudioController* ctrl = GetController();
	if (!ctrl || !ctrl->supported()) {
		return -1;
	}
	return ctrl->getVolume();
}

bool SetAudioVolume(int value)
{
	IAudioController* ctrl = GetController();
	if (!ctrl || !ctrl->supported()) {
		return false;
	}
	return ctrl->setVolume(value);
}
