//
// Core.h
//

#pragma once

#include "Keyboard.h"
#include <memory>

// Platform abstractions (forward declarations)
class IDeviceDetector;
class IDeviceDetectorListener;
class IKeyboardHandler;
class IAudioController;
class IResourceLocator;

class App;
class Config;
class SoundPlayer;
class Speech;

class Core : public IKeyEventListener
{
public:
    Core(App*, Config*, Device*);
    virtual ~Core();

    virtual bool OnKeyEvent(Key key, KeyEventType eventType, bool capsLock, bool shift, bool ctrl, bool alt) override;

    void OnClevyKeyboardConnected();
    void OnClevyKeyboardDisconnected();

    bool IsKeyboardConnected() const { return m_bKeyboardConnected; }

private:
    App* m_pApp;
    Config* m_pConfig;
    Keyboard* m_pKeyboard;
    // Platform abstraction instances (Phase 2 wiring)
    std::unique_ptr<IDeviceDetector> m_pDeviceDetector;
    // Keep the listener instance alive for the detector
    std::unique_ptr<IDeviceDetectorListener> m_pDeviceDetectorListener;
    std::unique_ptr<IKeyboardHandler> m_pPlatformKeyboardHandler;
    std::unique_ptr<IAudioController> m_pAudioController;
    std::unique_ptr<IResourceLocator> m_pResourceLocator;
    SoundPlayer* m_pSoundPlayer;
    Speech* m_pSpeech;

    std::string m_wordSpeechBuffer;
    std::string m_sentenceSpeechBuffer;

    bool m_bKeyboardConnected;
};
