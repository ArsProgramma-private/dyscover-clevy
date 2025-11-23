#include "DeviceDetector.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

namespace {
    constexpr PlatformCapabilities HOTPLUG_EVENTS = 1 << 0;

    class StubDeviceDetector : public IDeviceDetector {
    public:
        explicit StubDeviceDetector(IDeviceDetectorListener* listener) : m_listener(listener) {}
        bool isPresent() const override { return m_present.load(); }
        void startMonitoring() override {
            // announce current presence to the listener once
            if (m_listener) {
                m_listener->onDevicePresenceChanged(m_present.load());
            }
            // in a real implementation this would register for hotplug events
            m_monitoring = true;
        }
        void stopMonitoring() override { /* no-op stub */ }
        void refresh() override { /* no-op stub */ }
        PlatformCapabilities capabilities() const override { return HOTPLUG_EVENTS; }
    private:
        IDeviceDetectorListener* m_listener;
        std::atomic<bool> m_present{true};
        std::atomic<bool> m_monitoring{false};
    };

    // Polling wrapper: if a platform doesn't provide hotplug events this
    // wrapper will poll the underlying implementation and notify the listener
    // when presence changes.
    class PollingDeviceDetector : public IDeviceDetector {
    public:
        PollingDeviceDetector(std::unique_ptr<IDeviceDetector> impl, IDeviceDetectorListener* listener, unsigned intervalMs = 500)
            : m_impl(std::move(impl)), m_listener(listener), m_interval(intervalMs), m_running(false)
        {
            m_lastPresent = m_impl->isPresent();
        }

        ~PollingDeviceDetector() {
            stopMonitoring();
        }

        bool isPresent() const override { return m_impl->isPresent(); }

        void startMonitoring() override {
            if (m_running) return;
            m_running = true;
            // notify initial state
            if (m_listener) m_listener->onDevicePresenceChanged(m_lastPresent);
            m_thread = std::thread([this]() {
                while (m_running) {
                    bool current = m_impl->isPresent();
                    if (current != m_lastPresent) {
                        m_lastPresent = current;
                        if (m_listener) m_listener->onDevicePresenceChanged(current);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
                }
            });
        }

        void stopMonitoring() override {
            if (!m_running) return;
            m_running = false;
            if (m_thread.joinable()) m_thread.join();
        }

        void refresh() override {
            bool current = m_impl->isPresent();
            if (m_listener) m_listener->onDevicePresenceChanged(current);
        }

        PlatformCapabilities capabilities() const override {
            // Report that this implementation supports hotplug via polling
            return m_impl->capabilities() | (1 << 2); // set POLLING_ONLY bit
        }

    private:
        std::unique_ptr<IDeviceDetector> m_impl;
        IDeviceDetectorListener* m_listener{nullptr};
        unsigned m_interval{500};
        std::atomic<bool> m_running;
        std::thread m_thread;
        std::atomic<bool> m_lastPresent{false};
    };
}

std::unique_ptr<IDeviceDetector> CreateDeviceDetector(IDeviceDetectorListener* listener) {
#if defined(__USE_PLATFORM_DETECTOR__)
    // If the main application build opts into platform detectors, prefer the
    // platform implementation factory (provided by src/platform/* files).
    extern std::unique_ptr<IDeviceDetector> CreatePlatformDeviceDetector(IDeviceDetectorListener*);
    try {
        auto p = CreatePlatformDeviceDetector(listener);
        // if the platform implementation doesn't advertise HOTPLUG_EVENTS,
        // wrap it into a polling wrapper so the app still gets hotplug-like notifications.
        constexpr PlatformCapabilities HOTPLUG = 1 << 0;
        if ((p->capabilities() & HOTPLUG) == 0) {
            return std::unique_ptr<IDeviceDetector>(new PollingDeviceDetector(std::move(p), listener));
        }
        return p;
    } catch (...) {
        // fall back to stub on error
        return std::unique_ptr<IDeviceDetector>(new StubDeviceDetector(listener));
    }
#else
    // Default to the test-friendly stub implementation.
    return std::unique_ptr<IDeviceDetector>(new StubDeviceDetector(listener));
#endif
}
