# Contract: Runtime Method Switching

## Purpose
User can switch between methods (Classic/Dyscover, Default/Clevy Keyboard, KWeC) at runtime; key→sound/keystroke mappings update immediately.

## Inputs
- `Config::SetLayout(Layout)` from PreferencesDialog or tray.

## Behavior
- On change, active layout in LayoutRegistry is updated.
- Subsequent `TranslateKey(...)` calls use the newly active layout without restart.
- Selection persists across app restarts.

## Errors
- If requested layout not registered: log warning, keep previous active layout.

## Tests
- Given Classic → Default switch, `TranslateKey('A')` output differs accordingly.
- Persisted layout restored on restart.
