# Contract: Selection Read-Out (Speaker Key)

## Purpose
Pressing speaker key reads selected text from the active application.

## Inputs
- Key event: `Key::WinCmd` (down + release), `Config::GetSelection()`

## Behavior
- On trigger: send Ctrl+C, wait ~25ms, read clipboard text; if non-empty, set TTS speed from config and `Speak(text)`.
- ESC key stops ongoing speech.

## Errors
- Clipboard empty or inaccessible: no-op; optional notification.

## Tests
- Mock clipboard with known text; simulate trigger; assert `Speech::Speak` called with expected string.
