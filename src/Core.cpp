//
// Core.cpp
//

#include <wx/clipbrd.h>
#include <wx/log.h>
#include <wx/time.h>

#include "App.h"
#include "Config.h"
#include "Core.h"
#include "Keyboard.h"
#include "ResourceLoader.h"
#include "platform/DeviceDetector.h"
#include "platform/KeyboardHandler.h"
#include "platform/AudioController.h"
#include "platform/ResourceLocator.h"
#include "SoundPlayer.h"
#include "Speech.h"
#include "VersionInfo.h"

// T073: Logging normalization - subsystem tags
#define LOG_TAG_DEVICE "[Device] "
#define LOG_TAG_KEYBOARD "[Keyboard] "
#define LOG_TAG_AUDIO "[Audio] "
#define LOG_TAG_SPEECH "[Speech] "
#define LOG_TAG_CORE "[Core] "

Core::Core(App* pApp, Config* pConfig, Device* pDevice)
{
    m_pApp = pApp;
    m_pConfig = pConfig;
    // Integrated platform abstractions (T015 complete): retain legacy keyboard
    // only as fallback for translation/sending when platform handler lacks capability.
    m_pKeyboard = Keyboard::Create(this);
    m_pSoundPlayer = new SoundPlayer();
    m_pSpeech = new Speech();
    m_pSpeech->Init(GetTTSDataPath(), TTS_LANG, TTS_VOICE);
    m_pSpeech->SetVolume(RSTTS_VOLUME_MAX);

    // Device presence will be set by the detector's initial callback
    // during startMonitoring(), so initialize to false here
    m_bKeyboardConnected = false;

    // Instantiate platform abstractions (factories created in src/platform)
    // Device detector uses a listener adapter that calls Core's handlers
    struct DetectorAdapter : public IDeviceDetectorListener {
        DetectorAdapter(Core* c) : core(c) {}
        void onDevicePresenceChanged(bool present) override {
            if (present) core->OnClevyKeyboardConnected();
            else core->OnClevyKeyboardDisconnected();
        }
        Core* core;
    };

    // Create the actual platform-backed instances. These are currently stubs
    // and will be replaced with real implementations in later tasks. Keep the
    // listener owned by Core so it doesn't leak.
    try {
        m_pDeviceDetectorListener.reset(new DetectorAdapter(this));
        m_pDeviceDetector = CreateDeviceDetector(m_pDeviceDetectorListener.get());
        // Start monitoring for device hotplug events
        if (m_pDeviceDetector) {
            m_pDeviceDetector->startMonitoring();
            wxLogInfo(LOG_TAG_DEVICE "Device detector initialized and monitoring started");
        }
    } catch (...) {
        wxLogWarning(LOG_TAG_DEVICE "Failed to initialize device detector");
        m_pDeviceDetector.reset();
        m_pDeviceDetectorListener.reset();
    }

    try {
        m_pPlatformKeyboardHandler = CreateKeyboardHandler();
        if (m_pPlatformKeyboardHandler) {
            wxLogInfo(LOG_TAG_KEYBOARD "Platform keyboard handler initialized");
        }
    } catch (...) {
        wxLogWarning(LOG_TAG_KEYBOARD "Failed to initialize platform keyboard handler");
        m_pPlatformKeyboardHandler.reset();
    }

    try {
        m_pAudioController = CreateAudioController();
        if (m_pAudioController && m_pAudioController->supported()) {
            wxLogInfo(LOG_TAG_AUDIO "Audio controller initialized (backend: %d)", 
                      static_cast<int>(m_pAudioController->backend()));
        } else if (m_pAudioController) {
            wxLogWarning(LOG_TAG_AUDIO "Audio controller created but backend unsupported");
        }
    } catch (...) {
        wxLogWarning(LOG_TAG_AUDIO "Failed to initialize audio controller");
        m_pAudioController.reset();
    }

    try {
        m_pResourceLocator = CreateResourceLocator();
        if (m_pResourceLocator) {
            wxLogInfo(LOG_TAG_CORE "Resource locator initialized");
        }
    } catch (...) {
        wxLogWarning(LOG_TAG_CORE "Failed to initialize resource locator");
        m_pResourceLocator.reset();
    }
}

Core::~Core()
{
    // Stop device monitoring before cleanup
    if (m_pDeviceDetector) {
        m_pDeviceDetector->stopMonitoring();
    }
    
    m_pSpeech->Term();
    delete m_pSpeech;
    delete m_pSoundPlayer;
    delete m_pKeyboard;
}

bool Core::OnKeyEvent(Key key, KeyEventType eventType, bool capsLock, bool shift, bool ctrl, bool alt)
{
#ifdef __LICENSING_FULL__
    if (!m_bKeyboardConnected)  return false;
#endif

    if (!m_pConfig->GetEnabled())  return false;

    if (key == Key::WinCmd && eventType == KeyEventType::KeyDown && m_pConfig->GetSelection())
    {
        // Send Ctrl+C
        m_pKeyboard->SendKeyStroke(Key::C, false, true, false);

        // Wait a while
        wxMilliSleep(25);

        // Read text from clipboard and pronounce it
        if (wxTheClipboard->Open())
        {
            if (wxTheClipboard->IsSupported(wxDF_TEXT))
            {
                wxTextDataObject tdo;
                wxTheClipboard->GetData(tdo);
                wxString s = tdo.GetText();

                m_pSpeech->SetSpeed(static_cast<float>(m_pConfig->GetSpeed()));
                m_pSpeech->Speak(s.ToStdString());
            }

            wxTheClipboard->Close();
        }

        // Supress this event
        return true;
    }

    KeyTranslation translation = TranslateKey(key, capsLock, shift, ctrl, alt, m_pConfig->GetLayout());

    // Send simulated key strokes (prefer platform handler when available)
    if (eventType == KeyEventType::KeyDown)
    {
        for (KeyStroke ks : translation.keystrokes)
        {
            bool sent = false;
            if (m_pPlatformKeyboardHandler)
            {
                // Attempt platform send (currently single key only; modifiers handled implicitly later)
                sent = m_pPlatformKeyboardHandler->sendKey(ks.key, KeyEventType::KeyDown);
            }
            if (!sent)
            {
                // Fallback to legacy keyboard multi-key stroke injection
                m_pKeyboard->SendKeyStroke(ks.key, ks.shift, ks.ctrl, ks.alt);
            }
        }
    }

    // Play sound
    if (eventType == KeyEventType::KeyDown)
    {
        if (m_pConfig->GetLetters())
        {
            m_pSoundPlayer->StopPlaying();
            m_pSoundPlayer->PlaySoundFile(translation.sound);
        }
    }

    // Speech handling
    if (eventType == KeyEventType::KeyUp)
    {
        if (key == Key::Tab || key == Key::Space || key == Key::Enter)
        {
            if (!m_wordSpeechBuffer.empty() && m_pConfig->GetWords())
            {
                m_pSpeech->SetSpeed(static_cast<float>(m_pConfig->GetSpeed()));
                m_pSpeech->Speak(m_wordSpeechBuffer);
            }

            m_wordSpeechBuffer.clear();
            m_sentenceSpeechBuffer.push_back(' ');
        }
        else if (translation.speak_sentence)
        {
            m_pSpeech->SetSpeed(static_cast<float>(m_pConfig->GetSpeed()));

            if (!m_wordSpeechBuffer.empty() && m_pConfig->GetWords())
            {
                m_pSpeech->Speak(m_wordSpeechBuffer);
            }

            if (!m_sentenceSpeechBuffer.empty() && m_pConfig->GetSentences())
            {
                m_pSpeech->Speak(m_sentenceSpeechBuffer);
            }

            m_wordSpeechBuffer.clear();
            m_sentenceSpeechBuffer.clear();
        }
        else if (key == Key::Esc)
        {
            m_pSpeech->Stop();
        }
        else if (key == Key::Backspace)
        {
            if (!m_wordSpeechBuffer.empty())
            {
                m_wordSpeechBuffer.pop_back();
            }

            if (!m_sentenceSpeechBuffer.empty())
            {
                m_sentenceSpeechBuffer.pop_back();
            }
        }
        else
        {
            for (KeyStroke ks : translation.keystrokes)
            {
                std::string chars;
                if (m_pPlatformKeyboardHandler)
                {
                    KeyModifiers mods; mods.shift = ks.shift; mods.ctrl = ks.ctrl; mods.alt = ks.alt; // altGr unused
                    chars = m_pPlatformKeyboardHandler->translate(ks.key, mods);
                }
                if (chars.empty())
                {
                    chars = m_pKeyboard->TranslateKeyStroke(ks.key, ks.shift, ks.ctrl);
                }
                m_wordSpeechBuffer.append(chars);
                m_sentenceSpeechBuffer.append(chars);
            }
        }
    }

    // Supress all user keystrokes
    return true;
}

void Core::OnClevyKeyboardConnected()
{
    wxLogInfo(LOG_TAG_DEVICE "Clevy keyboard connected");
    m_pSoundPlayer->PlaySoundFile("dyscover_connect_positive_with_voice.wav");

    m_bKeyboardConnected = true;
}

void Core::OnClevyKeyboardDisconnected()
{
    wxLogInfo(LOG_TAG_DEVICE "Clevy keyboard disconnected");
    m_pSoundPlayer->PlaySoundFile("dyscover_connect_negative_with_voice.wav");

    m_bKeyboardConnected = false;
}
