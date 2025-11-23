// AudioControllerChromeOS.cpp - ChromeOS audio control
//
// ChromeOS runs in a sandboxed environment with restricted system access.
// Audio control via system APIs (PulseAudio, ALSA) is typically unavailable
// in the Crostini container or Chrome app sandbox.
//
// Implementation Strategy:
// - ChromeOS uses the same Linux kernel and libraries
// - The AudioControllerLinux.cpp implementation will be used
// - If audio libraries are unavailable, it returns Unsupported backend
// - No separate ChromeOS implementation needed
//
// Testing on ChromeOS:
// - If PulseAudio/ALSA inaccessible, returns Unsupported
// - Applications should check supported() before using audio control
// - UI should gracefully handle unsupported audio control
//
// Future Enhancement:
// - If Chrome Extensions Audio API becomes available, this file can be
//   extended with a ChromeOS-native implementation using Chrome APIs

// Intentionally empty - ChromeOS uses Linux audio controller implementation
