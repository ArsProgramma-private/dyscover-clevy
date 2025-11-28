# Phase 0 Research — Dyscover 4 Manual Compliance Audit

## Unknowns Extracted
- Runtime method switching (was compile-time): How to enable without heavy refactor?
- Tray icon three-state mapping: Asset availability and selection logic.
- Speaker key mapping: Confirm WinCmd as trigger across hardware variants.

## Findings & Decisions

### Decision 1: Runtime Method Switching
- Decision: Add runtime-selected layout in `LayoutRegistry` or a small indirection so `TranslateKey` uses `Config::GetLayout()`.
- Rationale: Manual requires immediate effect; UI already exposes selection.
- Approaches:
  1) Extend `LayoutRegistry` with `SetActiveLayout(name)` read by `TranslateKey`.
  2) Pass `Layout` from `Config` through call chain to `TranslateKey` and use a small dispatcher to the right provider.
- Choice: (1) Minimal surface change; update once in `PreferencesDialog::OnKeyboardMethodChanged` to set registry active layout.
- Alternatives: Keep compile-time selection (rejected; contradicts manual).

### Decision 2: Tray Icon Three-State Mapping
- Decision: Introduce 3-icon mapping (Active, Paused, No Keyboard (full)).
- Rationale: Manual lists distinct colors indicating state.
- Implementation: Add indices (e.g., 0=Active, 4=Paused, 5=No Keyboard) or new assets; centralize mapping in `TrayIcon::UpdateIcon`.
- Alternatives: Binary mapping (rejected; insufficient per manual).

### Decision 3: Speaker Key = WinCmd
- Decision: Keep `WinCmd` as the selection read and pronunciation modifier trigger.
- Rationale: Code already uses `Key::WinCmd` in `Core::OnKeyEvent`; legacy UI (frmMainUnit.h) references Speaker/SpeakerKey behavior.
- Alternatives: Dedicated HID scancode (rejected for now; adds complexity; can revisit if new hardware requires it).

## Test Strategy
- Unit: verify `TranslateKey` returns different outputs after runtime layout change.
- Unit: tray icon mapping returns expected index across (enabled/paused, keyboard present).
- Integration: simulate selection read (emulate clipboard) triggered by `WinCmd`.

## Risks
- Asset availability for paused/no keyboard icons (mitigate by reusing existing bundle indices or adding assets).
- Cross-platform behavior of Win key; documented as Windows requirement in spec.

