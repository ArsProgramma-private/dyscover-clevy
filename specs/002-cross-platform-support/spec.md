# Feature Specification: Cross-Platform Support (Linux, macOS, Windows, ChromeOS)

**Feature Branch**: `002-cross-platform-support`  
**Created**: November 23, 2025  
**Status**: Draft  
**Input**: User description: "we want to support linux, macos, windows and chromebooks. Find and fix code that hinders us from that"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Device Detection Works on All Platforms (Priority: P1)

Users can plug in a Clevy keyboard on any supported platform (Linux, macOS, Windows, ChromeOS) and the application correctly detects whether a supported device is present, triggering appropriate functionality.

**Why this priority**: Device detection is the core functionality that enables all other features. Without platform-agnostic device detection, the application cannot function on all target platforms.

**Independent Test**: Can be fully tested by installing the application on each target platform, connecting a Clevy keyboard, and verifying detection without any other features enabled. Delivers immediate value by confirming hardware compatibility.

**Acceptance Scenarios**:

1. **Given** a Windows system with the application installed, **When** a user connects a supported Clevy USB keyboard, **Then** the application detects the device and indicates it is present
2. **Given** a Linux system with the application installed, **When** a user connects a supported Clevy USB keyboard, **Then** the application detects the device using system device enumeration APIs
3. **Given** a macOS system with the application installed, **When** a user connects a supported Clevy USB keyboard, **Then** the application detects the device through the macOS device management system
4. **Given** a ChromeOS system with the application installed, **When** a user connects a supported Clevy USB keyboard, **Then** the application detects the device through available device APIs
5. **Given** any supported platform, **When** no Clevy keyboard is connected, **Then** the application correctly reports no supported device is present
6. **Given** a supported Clevy Bluetooth keyboard paired with any platform, **When** the keyboard connects, **Then** the application detects it correctly

---

### User Story 2 - Keyboard Input Handling Across Platforms (Priority: P2)

Users can interact with their Clevy keyboard on any platform, with the application correctly capturing key events, querying keyboard state (e.g., Caps Lock), simulating keystrokes, and translating key codes to characters according to the platform's keyboard layout.

**Why this priority**: Keyboard interaction is essential for the application's core accessibility features. After confirming device presence, enabling cross-platform keyboard handling allows the full feature set to work.

**Independent Test**: Can be tested independently by verifying keyboard event capture, state queries, and keystroke simulation work correctly on each platform without requiring device detection to be complete.

**Acceptance Scenarios**:

1. **Given** the application running on Windows, **When** a user presses keys on the Clevy keyboard, **Then** the application receives correct key events and can process them
2. **Given** the application running on Linux, **When** a user presses keys on the Clevy keyboard, **Then** the application receives and processes key events appropriately
3. **Given** the application running on macOS, **When** a user presses keys, **Then** key events are captured and processed correctly
4. **Given** the application running on ChromeOS, **When** keyboard input occurs, **Then** events are handled properly
5. **Given** any supported platform, **When** the application queries Caps Lock state, **Then** it receives the correct current state
6. **Given** any platform with a specific keyboard layout active, **When** the application translates a keystroke, **Then** it produces the correct character for that layout
7. **Given** the application needs to simulate a keystroke on any platform, **When** it sends a simulated key event, **Then** the system receives and processes it as if typed by the user

---

### User Story 3 - Audio Volume Control Works Cross-Platform (Priority: P3)

Users can have the application query and adjust system audio volume on any supported platform, enabling consistent audio feedback behavior regardless of operating system.

**Why this priority**: Audio control enhances user experience but is not critical for basic keyboard functionality. It can be implemented after core device and keyboard features are working.

**Independent Test**: Can be tested by verifying volume get/set operations work correctly on each platform independently of other features.

**Acceptance Scenarios**:

1. **Given** the application running on Windows, **When** it queries system volume, **Then** it receives the correct current volume level
2. **Given** the application running on Linux, **When** it queries system volume, **Then** it receives the current volume using Linux audio APIs
3. **Given** the application running on macOS, **When** it queries system volume, **Then** it receives the current volume through macOS audio services
4. **Given** the application running on ChromeOS, **When** it queries volume, **Then** it receives the current level through available audio interfaces
5. **Given** any platform, **When** the application sets system volume to a specific level, **Then** the system volume changes accordingly
6. **Given** audio APIs are unavailable on a platform, **When** the application attempts volume operations, **Then** it gracefully handles the limitation without crashing

---

### User Story 4 - Resources Load Correctly on All Platforms (Priority: P2)

The application can load icons, sound files, translations, and other resources on any supported platform, adapting to platform-specific file formats and path conventions.

**Why this priority**: Resource loading is essential for UI presentation and audio feedback. It's required for a complete user experience but doesn't block testing of core keyboard functionality.

**Independent Test**: Can be tested by launching the application on each platform and verifying all icons display, sounds play, and UI elements appear correctly.

**Acceptance Scenarios**:

1. **Given** the application running on Windows, **When** it loads tray icons, **Then** it uses ICO format resources correctly
2. **Given** the application running on macOS, **When** it loads tray icons, **Then** it uses ICNS format resources appropriately
3. **Given** the application running on Linux, **When** it loads icons, **Then** it uses appropriate icon formats for the desktop environment
4. **Given** the application running on ChromeOS, **When** resources are loaded, **Then** icons and assets display correctly
5. **Given** any platform, **When** the application needs to play a sound file, **Then** it locates the sound in the correct audio directory
6. **Given** any platform, **When** the application loads translations, **Then** it finds translation files using platform-appropriate paths

---

### Edge Cases

- What happens when device APIs are unavailable or restricted on a platform (e.g., ChromeOS security restrictions)?
- How does the system handle platforms where keyboard hooks are not available or require special permissions?
- What if audio APIs don't support volume control on a particular platform?
- How does the application behave when resource files are missing or inaccessible?
- What happens when USB device enumeration requires elevated permissions that the user hasn't granted?
- How does the system handle multiple Clevy keyboards connected simultaneously?
- What if the platform's device notification system works differently (e.g., polling vs. event-based)?

## Requirements *(mandatory)*

### Functional Requirements

#### Device Detection

- **FR-001**: System MUST detect supported Clevy keyboards on Windows using the Configuration Manager API
- **FR-002**: System MUST detect supported Clevy keyboards on Linux using libudev
- **FR-003**: System MUST detect supported Clevy keyboards on macOS using IOKit or appropriate device enumeration APIs
- **FR-004**: System MUST detect supported Clevy keyboards on ChromeOS using available device enumeration capabilities
- **FR-005**: System MUST support both USB and Bluetooth Clevy keyboard detection on all platforms where technically feasible
- **FR-006**: System MUST receive device connection/disconnection notifications on platforms that support them
- **FR-007**: System MUST fall back to polling-based detection on platforms without event-based device notifications

#### Keyboard Input Handling

- **FR-008**: System MUST capture low-level keyboard events on Windows using keyboard hooks
- **FR-009**: System MUST capture keyboard events on Linux using appropriate input subsystem APIs (e.g., X11, Wayland, evdev)
- **FR-010**: System MUST capture keyboard events on macOS using appropriate event monitoring APIs
- **FR-011**: System MUST capture keyboard events on ChromeOS through available input APIs
- **FR-012**: System MUST query Caps Lock state on all platforms
- **FR-013**: System MUST simulate keystroke injection on all platforms where security policies permit
- **FR-014**: System MUST translate key codes to character representations respecting active keyboard layouts on all platforms
- **FR-015**: System MUST handle keyboard input gracefully on platforms where low-level hooks are restricted

#### Audio Control

- **FR-016**: System MUST query system audio volume on Windows using Windows Multimedia API
- **FR-017**: System MUST query and control system audio volume on Linux using ALSA, PulseAudio, or PipeWire as appropriate
- **FR-018**: System MUST query and control system audio volume on macOS using Core Audio
- **FR-019**: System MUST query and control system audio volume on ChromeOS where API access permits
- **FR-020**: System MUST gracefully degrade functionality when volume control is unavailable on a platform

#### Resource Loading

- **FR-021**: System MUST load Windows icon resources from embedded RC files on Windows
- **FR-022**: System MUST load ICNS format icons from filesystem on macOS
- **FR-023**: System MUST load ICO or PNG format icons from filesystem on Linux
- **FR-024**: System MUST load appropriate icon formats on ChromeOS
- **FR-025**: System MUST locate sound files using platform-appropriate path resolution
- **FR-026**: System MUST locate translation files using platform-appropriate path conventions
- **FR-027**: System MUST load bitmap resources appropriate for each platform

#### Build System

- **FR-028**: Build system MUST compile platform-specific implementation files only for the target platform
- **FR-029**: Build system MUST link platform-specific libraries only when building for that platform
- **FR-030**: Build system MUST support cross-compilation where feasible
- **FR-031**: Build system MUST detect and report missing platform-specific dependencies at configure time

#### Code Structure

- **FR-032**: Platform-specific implementations MUST be isolated in separate source files
- **FR-033**: Common interfaces MUST be defined for all platform-specific functionality
- **FR-034**: System MUST use preprocessor directives or build system configuration to select platform implementations, not runtime checks
- **FR-035**: Shared code MUST avoid platform-specific API calls or data types

### Key Entities

- **Device**: Represents a Clevy keyboard (USB or Bluetooth), identified by vendor ID and product ID, with platform-specific detection state
- **Keyboard**: Represents keyboard input handling subsystem, maintains state (Caps Lock), processes events, simulates keystrokes
- **Audio Controller**: Represents system audio interface, tracks volume level, provides control operations
- **Resource Locator**: Manages platform-specific paths to icons, sounds, translations, and data files
- **Platform Abstraction Layer**: Common interfaces that hide platform differences from business logic

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Application successfully detects connected Clevy keyboards on 100% of supported platforms (Windows, Linux, macOS, ChromeOS) in manual testing
- **SC-002**: Application builds successfully for all four target platforms without errors or platform-specific compilation warnings
- **SC-003**: All keyboard input features (event capture, state query, keystroke translation) function correctly on at least 95% of tested platform configurations
- **SC-004**: Audio volume control works on at least 90% of platform configurations, with graceful degradation on others
- **SC-005**: Application launches and displays UI correctly (all icons and resources load) on 100% of supported platforms
- **SC-006**: Platform-specific code is isolated in separate files, with no platform-specific logic mixed into core business logic files (measurable via code review)
- **SC-007**: Application passes automated build and basic smoke tests on all four platforms in CI/CD pipeline
- **SC-008**: Device detection latency is under 500ms on all platforms when a keyboard is connected
- **SC-009**: Zero crashes reported related to missing platform-specific implementations during cross-platform testing

## Assumptions

- **A-001**: ChromeOS provides sufficient API access for device enumeration - if restricted, device detection may require alternative approaches or may not be fully supported
- **A-002**: All platforms have C++17-compatible development environments and required third-party libraries (wxWidgets, PortAudio) available
- **A-003**: macOS will use IOKit or modern device enumeration APIs available in recent macOS versions
- **A-004**: Linux implementation will prioritize udev-based detection as the primary method
- **A-005**: Keyboard hooks on Linux may require X11 or Wayland-specific implementations - we'll implement both or select based on runtime environment
- **A-006**: ChromeOS may impose security restrictions on low-level keyboard access - features requiring elevated privileges may be limited
- **A-007**: Audio control on Linux will support both ALSA and PulseAudio/PipeWire for broad compatibility
- **A-008**: Build system will use CMake's platform detection capabilities to select appropriate source files and libraries
