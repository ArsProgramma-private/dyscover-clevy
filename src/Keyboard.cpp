//
// Keyboard.cpp
//

#include "Keyboard.h"

#ifdef __PLATFORM_WINDOWS__
#include "KeyboardWindows.h"
#elif defined(__PLATFORM_LINUX__) || defined(__PLATFORM_MAC__)
#include "KeyboardLinux.h"
#endif

Keyboard* Keyboard::Create(IKeyEventListener* pListener)
{
#ifdef __PLATFORM_WINDOWS__
    Keyboard* pKeyboard = new KeyboardWindows(pListener);
#elif defined(__PLATFORM_LINUX__) || defined(__PLATFORM_MAC__)
    Keyboard* pKeyboard = new KeyboardLinux(pListener);
#endif

    pKeyboard->Initialize();

    return pKeyboard;
}

Keyboard::Keyboard(IKeyEventListener* pListener)
{
    m_pListener = pListener;

    m_bCapsLockActive = false;

    m_bShiftPressed = false;
    m_bCtrlPressed = false;
    m_bAltPressed = false;
}

void Keyboard::Initialize()
{
    m_bCapsLockActive = IsCapsLockActive();
}

void Keyboard::SendKeyStroke(Key key, bool shift, bool ctrl, bool alt)
{
    if (shift)
    {
        // Press Shift
        SendKeyEvent(KeyEventType::KeyDown, Key::Shift);
    }

    if (ctrl)
    {
        // Press Ctrl
        SendKeyEvent(KeyEventType::KeyDown, Key::Ctrl);
    }

    if (alt)
    {
        // Press Alt
        SendKeyEvent(KeyEventType::KeyDown, Key::Alt);
    }

    SendKeyEvent(KeyEventType::KeyDown, key);
    SendKeyEvent(KeyEventType::KeyUp, key);

    if (shift)
    {
        // Release Shift
        SendKeyEvent(KeyEventType::KeyUp, Key::Shift);
    }

    if (ctrl)
    {
        // Release Ctrl
        SendKeyEvent(KeyEventType::KeyUp, Key::Ctrl);
    }

    if (alt)
    {
        // Release Alt
        SendKeyEvent(KeyEventType::KeyUp, Key::Alt);
    }
}

bool Keyboard::ProcessKeyEvent(KeyEventType eventType, Key key)
{
    if (key == Key::CapsLock && eventType == KeyEventType::KeyDown)
    {
        m_bCapsLockActive = !m_bCapsLockActive;
    }

    if (key == Key::Shift)
    {
        m_bShiftPressed = eventType == KeyEventType::KeyDown;
    }

    if (key == Key::Ctrl)
    {
        m_bCtrlPressed = eventType == KeyEventType::KeyDown;
    }

    if (key == Key::Alt)
    {
        m_bAltPressed = eventType == KeyEventType::KeyDown;
    }

    if (key != Key::Unknown)
    {
        return m_pListener->OnKeyEvent(key, eventType, m_bCapsLockActive, m_bShiftPressed, m_bCtrlPressed, m_bAltPressed);
    }

    return false;
}
