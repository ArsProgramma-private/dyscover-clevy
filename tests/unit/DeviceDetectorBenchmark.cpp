// DeviceDetectorBenchmark.cpp - T038: Device detection latency benchmark
#include "platform/DeviceDetector.h"
#include <chrono>
#include <iostream>
#include <vector>

class BenchmarkListener : public IDeviceDetectorListener {
public:
    void onDevicePresenceChanged(bool present) override {
        (void)present; // Unused for benchmark
    }
};

int main() {
    std::cout << "Device Detection Latency Benchmark" << std::endl;
    std::cout << "===================================" << std::endl;
    
    BenchmarkListener listener;
    
    // Benchmark detector creation
    auto createStart = std::chrono::high_resolution_clock::now();
    auto detector = CreateDeviceDetector(&listener);
    auto createEnd = std::chrono::high_resolution_clock::now();
    
    auto createDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        createEnd - createStart).count();
    
    std::cout << "Detector creation time: " << createDuration << " μs" << std::endl;
    
    // Benchmark isPresent() call (initial detection)
    std::vector<long long> detectionTimes;
    const int iterations = 10;
    
    for (int i = 0; i < iterations; ++i) {
        auto detectionStart = std::chrono::high_resolution_clock::now();
        bool present = detector->isPresent();
        auto detectionEnd = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            detectionEnd - detectionStart).count();
        detectionTimes.push_back(duration);
        
        // Only print first iteration result
        if (i == 0) {
            std::cout << "Device present: " << (present ? "yes" : "no") << std::endl;
        }
    }
    
    // Calculate statistics
    long long sum = 0;
    long long min = detectionTimes[0];
    long long max = detectionTimes[0];
    
    for (auto time : detectionTimes) {
        sum += time;
        if (time < min) min = time;
        if (time > max) max = time;
    }
    
    long long avg = sum / iterations;
    
    std::cout << "\nDetection latency statistics (" << iterations << " iterations):" << std::endl;
    std::cout << "  Average: " << avg << " μs" << std::endl;
    std::cout << "  Minimum: " << min << " μs" << std::endl;
    std::cout << "  Maximum: " << max << " μs" << std::endl;
    
    // Benchmark refresh() operation
    auto refreshStart = std::chrono::high_resolution_clock::now();
    detector->refresh();
    auto refreshEnd = std::chrono::high_resolution_clock::now();
    
    auto refreshDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        refreshEnd - refreshStart).count();
    
    std::cout << "\nRefresh operation time: " << refreshDuration << " μs" << std::endl;
    
    // Benchmark monitoring start/stop
    auto monitorStart = std::chrono::high_resolution_clock::now();
    detector->startMonitoring();
    auto monitorStartEnd = std::chrono::high_resolution_clock::now();
    
    auto monitorStartDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        monitorStartEnd - monitorStart).count();
    
    std::cout << "Start monitoring time: " << monitorStartDuration << " μs" << std::endl;
    
    auto monitorStop = std::chrono::high_resolution_clock::now();
    detector->stopMonitoring();
    auto monitorStopEnd = std::chrono::high_resolution_clock::now();
    
    auto monitorStopDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        monitorStopEnd - monitorStop).count();
    
    std::cout << "Stop monitoring time: " << monitorStopDuration << " μs" << std::endl;
    
    // Check against performance requirement: <500ms (500,000 μs)
    const long long TARGET_LATENCY_US = 500000; // 500ms
    
    std::cout << "\nPerformance Assessment:" << std::endl;
    if (avg < TARGET_LATENCY_US) {
        std::cout << "✓ PASS: Average detection latency " << avg << " μs is below target " 
                  << TARGET_LATENCY_US << " μs (500ms)" << std::endl;
        return 0;
    } else {
        std::cout << "✗ FAIL: Average detection latency " << avg << " μs exceeds target " 
                  << TARGET_LATENCY_US << " μs (500ms)" << std::endl;
        return 1;
    }
}
