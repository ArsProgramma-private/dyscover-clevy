# Contract: Tray Icon State Mapping

## Purpose
Indicate state via icon: Active, Paused, and (full builds) No Keyboard.

## Inputs
- `Config::GetEnabled()`
- `App::IsClevyKeyboardPresent()` (only in __LICENSING_FULL__)

## Behavior
- Active: enabled = true and (full: keyboard present = true)
- Paused: enabled = false
- No Keyboard: (full) enabled = true and keyboard present = false
- Icon selection is deterministic and documented.

## Tests
- Simulate each permutation and assert selected icon index/asset.
