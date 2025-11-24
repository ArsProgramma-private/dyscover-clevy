// PerformanceSmokeTest.cpp - T074: Performance micro-benchmark (startup + detection latency)
#include "platform/DeviceDetector.h"
#include "platform/KeyboardHandler.h"
#include "platform/AudioController.h"
#include "platform/ResourceLocator.h"
#include <chrono>
#include <iostream>
#include <vector>

class TestListener : public IDeviceDetectorListener {
public:
    void onDevicePresenceChanged(bool present) override {
        (void)present;
    }
};

// Measure subsystem initialization time
struct InitBenchmark {
    std::string subsystem;
    long long duration_us;
};

int main() {
    std::cout << "Performance Smoke Test" << std::endl;
    std::cout << "======================" << std::endl;
    
    std::vector<InitBenchmark> results;
    
    // Benchmark device detector initialization
    TestListener listener;
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto detector = CreateDeviceDetector(&listener);
        auto end = std::chrono::high_resolution_clock::now();
        
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
        results.push_back({"DeviceDetector", duration});
    }
    
    // Benchmark keyboard handler initialization
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto handler = CreateKeyboardHandler();
        auto end = std::chrono::high_resolution_clock::now();
        
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
        results.push_back({"KeyboardHandler", duration});
    }
    
    // Benchmark audio controller initialization
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto controller = CreateAudioController();
        auto end = std::chrono::high_resolution_clock::now();
        
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
        results.push_back({"AudioController", duration});
    }
    
    // Benchmark resource locator initialization
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto locator = CreateResourceLocator();
        auto end = std::chrono::high_resolution_clock::now();
        
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
        results.push_back({"ResourceLocator", duration});
    }
    
    // Benchmark device detection operation
    {
        auto detector = CreateDeviceDetector(&listener);
        auto start = std::chrono::high_resolution_clock::now();
        bool present = detector->isPresent();
        auto end = std::chrono::high_resolution_clock::now();
        
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
        results.push_back({"Device Detection", duration});
        std::cout << "Device present: " << (present ? "yes" : "no") << std::endl;
    }
    
    // Display results
    std::cout << "\nSubsystem Initialization Times:" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    long long totalStartup = 0;
    for (const auto& result : results) {
        std::cout << result.subsystem << ": " << result.duration_us << " μs" << std::endl;
        // Only count initialization times (not the detection operation)
        if (result.subsystem != "Device Detection") {
            totalStartup += result.duration_us;
        }
    }
    
    std::cout << "\nTotal startup time: " << totalStartup << " μs ("
              << (totalStartup / 1000.0) << " ms)" << std::endl;
    
    // Performance targets from plan.md:
    // - Startup ≤2.0s p95
    // - Device detection latency <500ms
    // - Added abstraction <10ms overhead
    
    const long long TARGET_STARTUP_US = 10000; // 10ms overhead target
    const long long MAX_STARTUP_US = 2000000; // 2.0s absolute max
    
    std::cout << "\nPerformance Assessment:" << std::endl;
    
    bool pass = true;
    
    if (totalStartup > MAX_STARTUP_US) {
        std::cout << "✗ FAIL: Total startup " << totalStartup << " μs exceeds maximum "
                  << MAX_STARTUP_US << " μs (2.0s)" << std::endl;
        pass = false;
    } else if (totalStartup > TARGET_STARTUP_US) {
        std::cout << "⚠ WARNING: Total startup " << totalStartup << " μs exceeds target "
                  << TARGET_STARTUP_US << " μs (10ms), but within acceptable range" << std::endl;
    } else {
        std::cout << "✓ PASS: Total startup " << totalStartup << " μs is below target "
                  << TARGET_STARTUP_US << " μs (10ms)" << std::endl;
    }
    
    // Check detection latency
    for (const auto& result : results) {
        if (result.subsystem == "Device Detection") {
            const long long DETECTION_TARGET_US = 500000; // 500ms
            if (result.duration_us > DETECTION_TARGET_US) {
                std::cout << "✗ FAIL: Detection latency " << result.duration_us 
                          << " μs exceeds target " << DETECTION_TARGET_US << " μs (500ms)" << std::endl;
                pass = false;
            } else {
                std::cout << "✓ PASS: Detection latency " << result.duration_us 
                          << " μs is below target " << DETECTION_TARGET_US << " μs (500ms)" << std::endl;
            }
        }
    }
    
    return pass ? 0 : 1;
}
