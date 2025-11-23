// DeviceDetectorChromeOS.cpp - ChromeOS device detection
//
// ChromeOS is a restricted Linux environment that uses the same kernel subsystems
// but with enhanced sandboxing and security policies. For device detection:
//
// 1. USB device enumeration via libudev works the same as Linux
// 2. Hotplug notifications may be restricted in sandboxed contexts
// 3. The Linux detector implementation (DeviceDetectorLinux.cpp) is used
// 4. The polling fallback in DeviceDetector.cpp handles restricted scenarios
//
// Implementation Strategy:
// - ChromeOS builds use the same Linux detector implementation
// - The factory in DeviceDetector.cpp automatically wraps with polling if needed
// - No ChromeOS-specific code required due to kernel compatibility
//
// Build Configuration:
// - ChromeOS is detected as __linux__ at compile time
// - No separate ChromeOS detector class needed
// - DeviceDetectorLinux.cpp handles both standard Linux and ChromeOS
//
// Testing on ChromeOS:
// - If udev access is restricted, device enumeration returns empty list
// - The polling wrapper in CreateDeviceDetector provides fallback behavior
// - Apps gracefully degrade to polling mode if hotplug notifications fail
//
// Future Enhancement:
// - If ChromeOS-specific APIs become available (e.g., Chrome Extensions USB API),
//   this file can be extended with a ChromeOS-native implementation
// - Current approach ensures compatibility without ChromeOS-specific code

// Intentionally empty - ChromeOS uses Linux detector implementation
