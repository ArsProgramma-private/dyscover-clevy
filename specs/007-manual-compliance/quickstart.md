# Quickstart — Manual Compliance Validation

## Build

On Linux (debug):

```bash
# From repo root
./scripts/quick-build-linux.sh configure
./scripts/quick-build-linux.sh build
./scripts/quick-build-linux.sh test
```

On Windows (PowerShell):

```powershell
# From repo root
./win-quick-build.ps1 -Configure
./win-quick-build.ps1 -Build
./win-quick-build.ps1 -Test
```

## Validate Behaviors

1) Tray Icon (Three-State Mapping)
- **Icon states**: Icon index 0 (Active), 4 (Paused), 5 (No Keyboard in full builds).
- Start app; right-click tray; verify Enabled/Letters/Words/Sentences/Selection/Settings/Manual/Exit.
- Toggle Enabled and observe icon change (Active ↔ Paused).
- (Full build) Disconnect keyboard; observe No Keyboard icon (index 5).
- **Asset order**: `LoadDyscoverIcons()` loads Icon0..Icon6; mapping selects indices 0, 4, 5 per state.

2) Method Switching
- Open Settings; switch between Clevy Dyscover / Clevy Keyboard / KWeC.
- Type keys; verify sounds/keystrokes match selection immediately.

3) Selection Read-Out
- Select text in another app; press speaker key (Windows key).
- Verify the selected text is spoken; press ESC to stop.

4) Words/Sentences
- Enable both; type a sentence and end with period; words and sentence should be read.

5) Volume/Speed
- Adjust sliders; verify audible changes within 1s.

## Notes
- Speaker key mapping is Windows key (Win).
- Autostart option validated on Windows via reboot.
