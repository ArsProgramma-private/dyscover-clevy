# Data Model — Dyscover 4 Manual Compliance

## Entities

- Method
  - id: enum { Classic (Dyscover cover), Default (Clevy Keyboard), KWeC }
  - persisted: Config (`/Dyscover/Layout`)
  - behavior: selects active layout in LayoutRegistry

- AudioMapping
  - key: {Key, modifiers}
  - output: [KeyStroke]
  - sound: string (wav)
  - speak_sentence: bool
  - source: provided by layout provider (registry)

- SpeechConfig
  - volume: int (0..0xFFFF) via AudioController
  - speed: long (-25..+25) mapped to TTS speed
  - language: build config (TTS_LANG)
  - voice: build config (TTS_VOICE)

- DevicePresence
  - present: bool
  - source: DeviceDetector
  - effects: gates OnKeyEvent in full builds; tray icon state

- SelectionRead
  - enabled: bool (Config `/Dyscover/Selection`)
  - trigger: Key::WinCmd (speaker key)
  - behavior: copy selection → speak clipboard text

## Relationships
- Method → AudioMapping: one-to-many (selected method activates one set of mappings)
- SpeechConfig → Speech: config drives TTS instance speed/volume
- DevicePresence → Core/Tray: device presence influences gating and icon state

## Validation Rules
- Method must be one of allowed values per language build.
- Volume within controller-supported range; speed within [-25,+25].
- Selection read does nothing when selection empty or access fails.
