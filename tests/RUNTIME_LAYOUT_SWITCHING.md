# Test Documentation: Runtime Layout Switching

## Overview
Runtime layout switching enables the user to change keyboard methods (Clevy Dyscover/Clevy Keyboard/KWeC) via preferences UI, with immediate effect on key translation and audio playback.

## Implementation
- **Registry**: `src/layouts/LayoutRegistry` maintains an active layout override (`activeLayoutName_`).
- **Preferences**: `src/PreferencesDialog` calls `LayoutRegistry::Instance().SetActiveLayout(...)` on method change.
- **Startup**: `src/App` restores layout from `Config::GetLayout()` at startup.
- **Translation**: `src/Keys.cpp::TranslateKey` calls `LayoutRegistry::Instance().GetActiveLayout()` to retrieve the active provider.

## Tests

### Unit Tests
1. **LayoutSwitchingTest** (`tests/unit/LayoutSwitchingTest.cpp`)
   - **Purpose**: Verify that `SetActiveLayout` changes the active provider and affects `TranslateKey` output.
   - **Method**: Register two mock layouts with different sounds for Key::A; call `SetActiveLayout`; assert `TranslateKey` returns the expected sound.
   - **Status**: ✓ Passing

### Integration Opportunities
- **Preferences UI test**: Simulate method selection in `PreferencesDialog`; assert config is updated and active layout changes.
- **Startup restore test**: Set `Config::GetLayout()` to a specific method; restart app; assert active layout matches.

## Coverage
- Unit: Core registry and translation logic.
- Integration: End-to-end preferences → config → registry → translation flow (manual/E2E).

## Notes
- Each layout module registers itself at static initialization; ensure test suites link the required layout sources or use mock providers.
- The runtime override takes precedence over compile-time defaults (`__LAYOUT_*` macros).
