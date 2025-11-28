# Feature Specification: Dyscover 4 Manual Compliance Audit

**Feature Branch**: `007-manual-compliance`
**Created**: November 28, 2025  
**Status**: Draft  
**Input**: Validate that the software behavior matches the "Clevy Dyscover 4 – Manual" capabilities when matching hardware is present.

## User Scenarios & Testing (mandatory)

- **Tray control and status**: On start, a tray icon appears. Right-clicking shows a menu to enable/disable software, toggle Letters/Words/Sentences/Selection, open Settings, open Manual, and Exit. The icon reflects active/paused state (and, in full-licensed builds, whether a Clevy Keyboard is connected).  
  Test: Start the app; verify tray icon appears and menu items exist and toggle states persist.

- **Keyboard presence gating (Full license)**: When running a full-licensed build, Dyscover operates only if a Clevy Keyboard is detected.  
  Test: Connect/disconnect the keyboard; verify sounds/speech only occur when connected.

- **Method selection (cover/layout)**: In Settings, user selects method: Clevy Dyscover cover, Clevy Keyboard (no cover), or KWeC-over (Dutch).  
  Test: Switch method and verify key → sound/keystroke mapping uses the selected method.

- **Software Active toggle**: Pause/resume Dyscover without quitting.  
  Test: Toggle Enabled off; verify no interception/sounds/speech; toggle on to resume.

- **Automatic startup**: Option to start with the operating system (Windows requirement per manual).  
  Test: Enable auto-start; reboot; verify app starts automatically (Windows).

- **Letters**: When enabled, every keystroke produces the corresponding sound immediately.  
  Test: Press several keys; verify sounds play and immediate pronunciation (where applicable).

- **Words**: When enabled, each word is spoken after Space/Enter/Tab.  
  Test: Type a word and press Space; verify the word is spoken.

- **Sentences**: When enabled, sentences are spoken after punctuation (., !, ?) or Enter at sentence end.  
  Test: Type a sentence and end with a period; verify the sentence is spoken.

- **Selection read-out (speaker key)**: Selecting text and pressing the cover’s speaker key reads the selection aloud.  
  Test: Select text in another application and press the speaker key; verify selected text is spoken.

- **Volume and speed**: Sliders control system volume and TTS speech speed.  
  Test: Adjust sliders; verify effect on output volume and speaking rate.

- **Stop speech (Esc)**: ESC key stops ongoing TTS.  
  Test: Start long read; press Esc; verify speech stops immediately.

- **Windows-wide TTS**: Text-to-Speech works across Microsoft Windows applications where text can be entered.  
  Test: Enable Words/Sentences; type into Notepad/Word; verify read-outs occur appropriately.

- **Alternate pronunciations (Clevy key)**: Holding the special Clevy key modifies pronunciation of certain letters (e.g., ‘C’ as K vs. S).  
  Test: Hold the modifier and press relevant keys; verify alternate pronunciation is used; pressing without modifier uses default.

If any flow is unsupported by the build configuration (OS, license mode, language), mark the corresponding scenario as Not Applicable in testing notes.

### Edge Cases

- Keyboard disconnects/reconnects while speaking a sentence: speech should complete or stop per policy but inputs must be gated immediately when disconnected (full build).
- Selection read-out when clipboard access is denied or selection is empty: provide a non-intrusive notification and no speech output.
- Words/Sentences disabled while buffers are non-empty: buffers should clear without emitting speech.
- Volume at minimum or muted system: maintain visual confirmation in settings; no speech should be audible.

## Functional Requirements (mandatory)

- **FR-001 Tray**: Application shows a system tray icon with a context menu containing Enabled, Letters, Words, Sentences, Selection, Settings, Manual, Exit.
- **FR-002 Icon state**: Tray icon uses distinct visuals for these states: Active, Paused, and (in full-licensed builds) No Keyboard. The mapping is stable and documented in code comments and release notes.
- **FR-003 Keyboard gating**: In full-licensed builds, application processes input only when a supported Clevy Keyboard is detected; otherwise remains idle.
- **FR-004 Method selection**: Settings provide a runtime method selector with options: Clevy Dyscover (cover), Clevy Keyboard (no cover), and KWeC-over (Dutch). The selection takes effect immediately at runtime (no restart), persists across restarts, and determines key→keystroke and key→sound mappings.
- **FR-005 Active toggle**: When Disabled, the application does not intercept keys nor produce sounds/speech; re-enabling restores behavior without restart.
- **FR-006 Autostart (Windows)**: Users can enable automatic startup with the operating system (Windows) via Settings; respects toggle.
- **FR-007 Letters**: When Letters is enabled, pressing a key plays its configured sound immediately.
- **FR-008 Words**: When Words is enabled, the word buffer is spoken upon Space/Enter/Tab.
- **FR-009 Sentences**: When Sentences is enabled, the sentence buffer is spoken upon sentence-ending punctuation or Enter.
- **FR-010 Selection read**: When Selection is enabled and the user presses the cover’s speaker key (mapped to the Windows key / `WinCmd`), the application copies the current selection and speaks it.
- **FR-011 Volume**: The volume slider adjusts output volume within the system/device audio controller’s supported range.
- **FR-012 Speed**: The speed slider adjusts TTS speech rate within supported bounds.
- **FR-013 Stop (Esc)**: Pressing ESC stops any ongoing speech playback immediately.
- **FR-014 Windows-wide TTS**: TTS speech triggers for words/sentences while typing in standard Windows applications (where text input is accepted).
- **FR-015 Alternate pronunciations**: Holding the Clevy key modifies pronunciation logic for designated letters/sounds; pressing the key normally uses the default pronunciation.
- **FR-016 Manual link**: The Manual option opens the correct language manual page in the default browser.

## Success Criteria (mandatory)

- **SC-001 Discoverability**: 95% of test users can locate and open the tray menu within 10 seconds.
- **SC-002 Accuracy**: 100% of letter/word/sentence triggers produce corresponding audio/TTS per selected method in supported language(s).
- **SC-003 Cross-app**: In Windows apps (Notepad, Word, browser inputs), 95%+ of tested typing sequences produce correct word/sentence read-outs when enabled.
- **SC-004 Control**: ESC stops speech within 200 ms; volume/speed adjustments reflect within 1 second of change.
- **SC-005 Keyboard gating**: In full builds, no Dyscover outputs occur while the Clevy keyboard is disconnected; outputs resume within 1 second of reconnection.
- **SC-006 Persistence**: Changes to Enabled/Letters/Words/Sentences/Selection, method, volume, speed persist across application restarts.
- **SC-007 Startup**: When Autostart is enabled on Windows, app is present in Startup and launches on reboot.
- **SC-008 Selection reading**: Selected text is correctly read in 3 major apps (Word, browser, PDF reader with selectable text) when speaker key is pressed.

## Key Entities

- **Method**: Runtime setting determining layout and sound mapping (Dyscover cover, Clevy Keyboard, KWeC-over).
- **Audio Mapping**: Mapping between keys (with modifiers) and audio file(s), including flags to indicate sentence boundaries.
- **Speech Engine Config**: Language, voice, speed, and volume parameters applied to TTS synthesis.
- **Device Presence**: Boolean state indicating whether a supported Clevy Keyboard is connected.

## Assumptions

- The manual’s “speaker key” maps to the Windows key (`WinCmd`) on supported platforms and is uniquely distinguishable in software.
- Autostart behavior is only required on Windows; non-Windows platforms may store a preference without OS-level startup integration.
- Supported languages per current product scope are Dutch (NL) and Flemish (NL_BE); KWeC is a Dutch-only method.

## Clarifications Resolved

- Runtime method switching is required. Users must be able to toggle between Clevy Dyscover, Clevy Keyboard, and KWeC at runtime with immediate effect and persistence.
- Tray icon must represent three distinct states: Active, Paused, and (full-licensed builds) No Keyboard.
- The “speaker/Clevy key” is mapped to the Windows key (`WinCmd`) for selection read-out and alternate pronunciations.

## Acceptance Mapping (evidence outline)

This section outlines how each manual capability is addressed by requirements and how it will be validated during QA:

- Tray menu and states → FR-001, FR-002; Tests in “Tray control and status”
- Keyboard presence gating (full) → FR-003; Tests in “Keyboard presence gating”
- Method selection → FR-004; Tests in “Method selection”
- Enabled toggle → FR-005; Tests in “Software Active toggle”
- Autostart → FR-006; Tests in “Automatic startup”
- Letters/Words/Sentences → FR-007/008/009; Tests under respective scenarios
- Selection read-out → FR-010; Tests in “Selection read-out (speaker key)”
- Volume/Speed → FR-011/012; Tests in “Volume and speed”
- Stop speech (Esc) → FR-013; Tests in “Stop speech (Esc)”
- Windows-wide TTS → FR-014; Tests in “Windows-wide TTS”
- Alternate pronunciations (Clevy key) → FR-015; Tests in “Alternate pronunciations”
- Manual link → FR-016; Tests in “Tray control and status”
