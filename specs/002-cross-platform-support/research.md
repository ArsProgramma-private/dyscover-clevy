# Phase 0 Research: Cross-Platform Support

Date: 2025-11-23  
Branch: 002-cross-platform-support  
Spec: spec.md  
Plan: plan.md

## Research Decisions

| Item # | Topic | Decision | Rationale | Alternatives Considered | Risk / Mitigation |
|--------|-------|----------|-----------|--------------------------|-------------------|
| 1 | macOS VID/PID enumeration | Use IOKit IORegistry via `IOServiceGetMatchingServices` matching `kIOUSBDeviceClassName`; extract `idVendor`, `idProduct` | Direct stable API; widely documented; supports both Intel & Apple Silicon | HID Manager (higher-level, may abstract devices); libusb (extra dependency) | Medium: API evolution → Mitigate with wrapper + compile-time availability checks |
| 2 | macOS keyboard hook | Use CGEventTap (Quartz Event Tap) at `kCGHIDEventTap` | Allows low-level interception + filtering; standard for key remapping tools | NSEventMonitor (app-level only), IOHIDQueue (complex) | Permissions: requires Accessibility access → Mitigate with user prompt & fallback to no interception |
| 3 | ChromeOS device access | Treat as Linux variant using udev; if sandboxed restricts `/dev`, fallback to periodic polling of existing devices | Minimizes divergence; leverages Linux code path | Separate ChromeOS API abstraction | High (sandbox) → detect failures & surface limited support notice |
| 4 | Linux volume backend | Prefer PulseAudio/PipeWire via libpulse; fallback to ALSA (`snd_mixer`) if libpulse unavailable | Broad coverage for modern distros; layered fallback | External `pactl` invocation (process overhead), PipeWire direct API (adds dependency) | Dual-path complexity → Isolate backends behind single class; unit tests for selection logic |
| 5 | ChromeOS volume | Attempt libpulse path; if unavailable, mark volume set unsupported (read returns cached value) | Simplify; consistent user messaging | Implement ChromeOS-specific dbus calls (overkill) | Low: user confusion → mitigate with UI message when volume control disabled |
| 6 | Apple Silicon build | Initial target x86_64 only; add universal binary after stabilization | Reduces immediate complexity; ensures faster iteration | Universal binary first | Medium: M1 users impacted → Document in README; plan follow-up feature |
| 7 | Abstraction namespace | Use `platform/` directory with plain headers in global namespace; prefix classes `Platform` optional NOT USED | Avoid deep namespaces; reduces churn; consistent with existing style | Introduce nested namespaces (e.g., `dyscover::platform`) | Low: potential future collisions → revisit if needed |
| 8 | Testing without hardware | Provide mock enumerator returning synthetic VID/PID list; dependency injection via factory | Enables deterministic unit tests & CI matrix | Skip tests on non-target platforms (reduced coverage) | Low: test realism → Add integration tests gated behind flag |
| 9 | Keystroke translation | Abstract per-platform using existing Windows logic; Linux/macOS use layout-aware APIs: XKB on Linux, TISInputSource on macOS | Correct layout mapping; handles dead keys better than naive ASCII | Manual mapping tables (maintenance heavy) | Medium: multiple APIs → encapsulate; fallback returns empty on failure |
|10 | Keystroke injection permissions | Graceful degradation: if injection blocked (macOS Accessibility / ChromeOS security), operations return false without throwing | Avoids crash; clear user messaging | Attempt privilege escalation or complex entitlements | Medium: feature gap → log & document; track in complexity table with expiry |

## Consolidated Decisions Summary
- macOS device: IOKit enumeration (IORegistry) wrapper class `MacDeviceEnumerator`
- macOS keyboard: CGEventTap; fallback: no interception (still detection if possible)
- Linux volume: libpulse primary; ALSA mixer fallback; selection at runtime
- ChromeOS: reuse Linux code; detect sandbox failures; degrade gracefully
- Keystroke translation: XKB (Linux) & TISInputSource (macOS); Windows remains
- Injection fallback: capability probing; disabled path logs once per session
- Testing: mock factories for device, keyboard, audio; integration tests optional
- Build: initial x86_64 target across OS; Apple Silicon later

## Dependency Impact
- New external APIs: IOKit (system), CoreAudio (system), libpulse (likely present), ALSA (header presence optional)
- No additional third-party libraries beyond system frameworks and existing dependencies.

## Open Risks After Research
| Risk | Description | Impact | Mitigation |
|------|-------------|--------|-----------|
| Sandbox limitations ChromeOS | udev access blocked | Device detection fails | Poll limited HID interfaces; user notice |
| Accessibility permission macOS | Keyboard interception blocked | Key filtering disabled | Detect lack of permission; prompt user; fallback |
| libpulse absent on minimal Linux | Volume operations fail | Reduced audio control | ALSA fallback; detect at init |
| XKB availability on headless systems | Translation inaccurate | Character mapping lost | Skip translation tests; return raw key name |

## Alternatives Rejected Detailed Notes
- HID Manager for macOS device enumeration: higher-level but adds complexity for matching vendor/product; IOKit offers direct property access.
- External `pactl` processes: increase latency and error handling burden; prefer native API calls.
- Manual translation tables: high maintenance and fragile with locale variations.

## Next Phase Inputs
Use interface list & backend decisions to define `data-model.md` entities and contracts. Constitution re-check will confirm test coverage strategy & complexity compliance.
