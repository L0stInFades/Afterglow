#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <memory>

namespace UltraImageViewer {
namespace Utils {

/**
 * High-resolution performance timer
 */
class PerformanceTimer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double>;

    PerformanceTimer();
    void Start();
    void Stop();
    void Reset();

    double GetElapsedSeconds() const;
    double GetElapsedMilliseconds() const;
    double GetElapsedMicroseconds() const;

private:
    TimePoint startTime_;
    TimePoint endTime_;
    bool running_ = false;
};

/**
 * Performance statistics collector
 */
class PerformanceStats {
public:
    std::string name;
    std::atomic<size_t> count{0};
    std::atomic<double> totalTime{0.0};
    std::atomic<double> minTime{1e30};
    std::atomic<double> maxTime{0.0};

    void Record(double milliseconds);
    double GetAverage() const;
    double GetMin() const;
    double GetMax() const;
    size_t GetCount() const;
    void Reset();
};

/**
 * Performance monitor for tracking application metrics
 * Tracks FPS, memory usage, decoding times, etc.
 */
class PerformanceMonitor {
public:
    struct Metrics {
        // Rendering
        double currentFPS = 0.0;
        double averageFPS = 0.0;
        double minFPS = 60.0;
        double maxFPS = 0.0;
        size_t frameCount = 0;

        // Memory
        size_t currentMemoryUsage = 0;
        size_t peakMemoryUsage = 0;
        size_t gpuMemoryUsage = 0;

        // Image operations
        double averageDecodeTime = 0.0;
        double averageUploadTime = 0.0;
        size_t imagesDecoded = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;

        // GPU
        double averageGPUTime = 0.0;
        size_t drawCalls = 0;
    };

    PerformanceMonitor();
    ~PerformanceMonitor();

    // Frame timing
    void BeginFrame();
    void EndFrame();

    // Custom timers
    void StartTimer(const std::string& name);
    void StopTimer(const std::string& name);

    // Recording
    void RecordMetric(const std::string& name, double value);
    void RecordDecodeTime(double milliseconds);
    void RecordUploadTime(double milliseconds);
    void RecordGPUTime(double milliseconds);

    // Statistics
    PerformanceStats* GetStats(const std::string& name);
    Metrics GetCurrentMetrics() const;
    void Reset();

    // Memory tracking
    void UpdateMemoryUsage();
    size_t GetCurrentMemoryUsage() const;
    size_t GetPeakMemoryUsage() const;

    // FPS calculation
    double GetCurrentFPS() const;
    double GetAverageFPS() const;

    // Logging
    void LogStats();
    std::string GenerateReport();

private:
    // Timers
    std::unordered_map<std::string, std::unique_ptr<PerformanceTimer>> timers_;
    std::unordered_map<std::string, std::unique_ptr<PerformanceStats>> stats_;

    // Frame timing
    PerformanceTimer frameTimer_;
    std::vector<double> fpsHistory_;
    static constexpr size_t FPS_HISTORY_SIZE = 60;

    // Metrics
    Metrics currentMetrics_;

    // Memory
    size_t baselineMemory_ = 0;

    // FPS calculation
    void UpdateFPS();
};

/**
 * Scoped timer for automatic timing
 */
class ScopedTimer {
public:
    ScopedTimer(PerformanceMonitor* monitor, const std::string& name);
    ~ScopedTimer();

private:
    PerformanceMonitor* monitor_;
    std::string name_;
};

/**
 * Memory usage tracker
 */
class MemoryTracker {
public:
    struct MemorySnapshot {
        size_t workingSetSize = 0;
        size_t peakWorkingSetSize = 0;
        size_t pagefileUsage = 0;
        size_t privateBytes = 0;

        size_t GetTotal() const;
    };

    static MemorySnapshot GetSnapshot();
    static size_t GetCurrentUsage();
    static size_t GetPeakUsage();

    // Process memory
    static bool GetProcessMemoryInfo(MemorySnapshot& snapshot);
};

/**
 * GPU memory tracker
 */
class GPUMemoryTracker {
public:
    static size_t EstimateGPUMemory(size_t width, size_t height, size_t bytesPerPixel);
    static size_t EstimateMipMemory(size_t width, size_t height, size_t bytesPerPixel, size_t levels);
};

} // namespace Utils
} // namespace UltraImageViewer
