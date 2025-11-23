# Contracts: Interface Definitions (Pseudo-Code)

## DeviceDetector
```cpp
class IDeviceDetectorListener {
public:
    virtual void onDevicePresenceChanged(bool present) = 0;
    virtual ~IDeviceDetectorListener() = default;
};

class IDeviceDetector {
public:
    virtual bool isPresent() const = 0;
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
    virtual void refresh() = 0;
    virtual PlatformCapabilities capabilities() const = 0;
    virtual ~IDeviceDetector() = default;
};

std::unique_ptr<IDeviceDetector> createDeviceDetector(IDeviceDetectorListener* listener);
```

## KeyboardHandler
```cpp
struct KeyModifiers { bool shift; bool ctrl; bool alt; bool altGr; };

class IKeyboardHandler {
public:
    virtual bool isCapsLockActive() const = 0;
    virtual std::string translate(Key key, const KeyModifiers& mods) = 0;
    virtual bool sendKey(Key key, KeyEventType type) = 0; // false if unsupported
    virtual void startInterception() = 0; // may throw or no-op if permission denied
    virtual void stopInterception() = 0;
    virtual PermissionState permissionState() const = 0;
    virtual ~IKeyboardHandler() = default;
};

std::unique_ptr<IKeyboardHandler> createKeyboardHandler(IKeyEventListener* listener);
```

## AudioController
```cpp
class IAudioController {
public:
    virtual int getVolume() = 0; // 0-65535, -1 on error
    virtual bool setVolume(int value) = 0; // false if unsupported
    virtual BackendType backend() const = 0;
    virtual bool supported() const = 0;
    virtual ~IAudioController() = default;
};

std::unique_ptr<IAudioController> createAudioController();
```

## ResourceLocator
```cpp
class IResourceLocator {
public:
    virtual std::string iconFile(const std::string& baseName) const = 0;
    virtual std::string splashBitmap() const = 0;
    virtual std::string audioDir() const = 0;
    virtual std::string ttsDir() const = 0;
    virtual std::string translationsDir() const = 0;
    virtual ~IResourceLocator() = default;
};

std::unique_ptr<IResourceLocator> createResourceLocator();
```

## PlatformUtils (Header Sketch)
```cpp
namespace PlatformUtils {
    bool extractVidPid(const std::string& hardwareId, std::string& vid, std::string& pid);
    std::string platformName();
}
```

## Error & Permission Enums
```cpp
enum class PermissionState { Unknown, Granted, Denied, Limited };

enum PlatformCapability : uint32_t {
    HOTPLUG_EVENTS = 1 << 0,
    BLUETOOTH      = 1 << 1,
    POLLING_ONLY   = 1 << 2
};
using PlatformCapabilities = uint32_t;
```

## Testing Contracts
- Mocks will implement each interface; injection via factory functions.
- Factories responsible for selecting concrete implementation based on compile-time platform macros (`_WIN32`, `__APPLE__`, `__linux__`).

