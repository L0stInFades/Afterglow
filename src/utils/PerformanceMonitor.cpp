#include "utils/PerformanceMonitor.hpp"
#include <windows.h>
#include <psapi.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

#pragma comment(lib, "psapi.lib")

namespace UltraImageViewer {
namespace Utils {

// PerformanceTimer implementation
PerformanceTimer::PerformanceTimer()
    : startTime_()
    , endTime_()
    , running_(false)
{
}

void PerformanceTimer::Start()
{
    startTime_ = Clock::now();
    running_ = true;
}

void PerformanceTimer::Stop()
{
    endTime_ = Clock::now();
    running_ = false;
}

void PerformanceTimer::Reset()
{
    startTime_ = TimePoint();
    endTime_ = TimePoint();
    running_ = false;
}

double PerformanceTimer::GetElapsedSeconds() const
{
    auto end = running_ ? Clock::now() : endTime_;
    return std::chrono::duration_cast<Duration>(end - startTime_).count();
}

double PerformanceTimer::GetElapsedMilliseconds() const
{
    return GetElapsedSeconds() * 1000.0;
}

double PerformanceTimer::GetElapsedMicroseconds() const
{
    return GetElapsedSeconds() * 1000000.0;
}

// PerformanceStats implementation
void PerformanceStats::Record(double milliseconds)
{
    count++;
    totalTime += milliseconds;

    double currentMin = minTime;
    double currentMax = maxTime;

    // Update min/max atomically
    double desiredMin = std::min(currentMin, milliseconds);
    double desiredMax = std::max(currentMax, milliseconds);

    minTime.store(desiredMin, std::memory_order_relaxed);
    maxTime.store(desiredMax, std::memory_order_relaxed);
}

double PerformanceStats::GetAverage() const
{
    size_t countValue = count;
    return countValue > 0 ? totalTime / countValue : 0.0;
}

double PerformanceStats::GetMin() const
{
    return count > 0 ? minTime : 0.0;
}

double PerformanceStats::GetMax() const
{
    return maxTime;
}

size_t PerformanceStats::GetCount() const
{
    return count;
}

void PerformanceStats::Reset()
{
    count = 0;
    totalTime = 0.0;
    minTime = 1e30;
    maxTime = 0.0;
}

// PerformanceMonitor implementation
PerformanceMonitor::PerformanceMonitor()
{
    fpsHistory_.reserve(FPS_HISTORY_SIZE);
    baselineMemory_ = MemoryTracker::GetCurrentUsage();
    frameTimer_.Start();
}

PerformanceMonitor::~PerformanceMonitor() = default;

void PerformanceMonitor::BeginFrame()
{
    frameTimer_.Reset();
    frameTimer_.Start();
}

void PerformanceMonitor::EndFrame()
{
    frameTimer_.Stop();
    double frameTime = frameTimer_.GetElapsedMilliseconds();

    // Calculate FPS
    if (frameTime > 0.0) {
        double fps = 1000.0 / frameTime;
        fpsHistory_.push_back(fps);

        if (fpsHistory_.size() > FPS_HISTORY_SIZE) {
            fpsHistory_.erase(fpsHistory_.begin());
        }

        UpdateFPS();
    }

    currentMetrics_.frameCount++;
}

void PerformanceMonitor::StartTimer(const std::string& name)
{
    auto it = timers_.find(name);
    if (it == timers_.end()) {
        timers_[name] = std::make_unique<PerformanceTimer>();
        it = timers_.find(name);
    }

    it->second->Start();
}

void PerformanceMonitor::StopTimer(const std::string& name)
{
    auto it = timers_.find(name);
    if (it != timers_.end()) {
        it->second->Stop();

        // Record in stats
        double elapsed = it->second->GetElapsedMilliseconds();
        RecordMetric(name, elapsed);
    }
}

void PerformanceMonitor::RecordMetric(const std::string& name, double value)
{
    auto it = stats_.find(name);
    if (it == stats_.end()) {
        stats_[name] = std::make_unique<PerformanceStats>();
        stats_[name]->name = name;
        it = stats_.find(name);
    }

    it->second->Record(value);
}

void PerformanceMonitor::RecordDecodeTime(double milliseconds)
{
    RecordMetric("DecodeTime", milliseconds);
    currentMetrics_.imagesDecoded++;
    currentMetrics_.averageDecodeTime = GetStats("DecodeTime")->GetAverage();
}

void PerformanceMonitor::RecordUploadTime(double milliseconds)
{
    RecordMetric("UploadTime", milliseconds);
    currentMetrics_.averageUploadTime = GetStats("UploadTime")->GetAverage();
}

void PerformanceMonitor::RecordGPUTime(double milliseconds)
{
    RecordMetric("GPUTime", milliseconds);
    currentMetrics_.averageGPUTime = GetStats("GPUTime")->GetAverage();
}

PerformanceStats* PerformanceMonitor::GetStats(const std::string& name)
{
    auto it = stats_.find(name);
    if (it != stats_.end()) {
        return it->second.get();
    }
    return nullptr;
}

PerformanceMonitor::Metrics PerformanceMonitor::GetCurrentMetrics() const
{
    return currentMetrics_;
}

void PerformanceMonitor::Reset()
{
    for (auto& pair : stats_) {
        pair.second->Reset();
    }

    currentMetrics_ = Metrics();
    fpsHistory_.clear();
    frameTimer_.Reset();
}

void PerformanceMonitor::UpdateMemoryUsage()
{
    size_t currentUsage = MemoryTracker::GetCurrentUsage() - baselineMemory_;
    currentMetrics_.currentMemoryUsage = currentUsage;
    currentMetrics_.peakMemoryUsage = std::max(currentMetrics_.peakMemoryUsage, currentUsage);
}

size_t PerformanceMonitor::GetCurrentMemoryUsage() const
{
    return currentMetrics_.currentMemoryUsage;
}

size_t PerformanceMonitor::GetPeakMemoryUsage() const
{
    return currentMetrics_.peakMemoryUsage;
}

double PerformanceMonitor::GetCurrentFPS() const
{
    return currentMetrics_.currentFPS;
}

double PerformanceMonitor::GetAverageFPS() const
{
    return currentMetrics_.averageFPS;
}

void PerformanceMonitor::UpdateFPS()
{
    if (fpsHistory_.empty()) {
        currentMetrics_.currentFPS = 0.0;
        return;
    }

    // Current FPS (most recent frame)
    currentMetrics_.currentFPS = fpsHistory_.back();

    // Average FPS
    double sum = std::accumulate(fpsHistory_.begin(), fpsHistory_.end(), 0.0);
    currentMetrics_.averageFPS = sum / fpsHistory_.size();

    // Min/Max FPS
    currentMetrics_.minFPS = *std::min_element(fpsHistory_.begin(), fpsHistory_.end());
    currentMetrics_.maxFPS = *std::max_element(fpsHistory_.begin(), fpsHistory_.end());
}

void PerformanceMonitor::LogStats()
{
    // Output to debug console
    OutputDebugStringA(GenerateReport().c_str());
}

std::string PerformanceMonitor::GenerateReport()
{
    std::ostringstream oss;

    oss << "=== UltraImageViewer Performance Report ===\n";
    oss << "\nRendering:\n";
    oss << "  Current FPS: " << std::fixed << std::setprecision(1) << currentMetrics_.currentFPS << "\n";
    oss << "  Average FPS: " << currentMetrics_.averageFPS << "\n";
    oss << "  Min FPS: " << currentMetrics_.minFPS << "\n";
    oss << "  Max FPS: " << currentMetrics_.maxFPS << "\n";
    oss << "  Frames: " << currentMetrics_.frameCount << "\n";

    oss << "\nMemory:\n";
    oss << "  Current: " << (currentMetrics_.currentMemoryUsage / 1024 / 1024) << " MB\n";
    oss << "  Peak: " << (currentMetrics_.peakMemoryUsage / 1024 / 1024) << " MB\n";
    oss << "  GPU: " << (currentMetrics_.gpuMemoryUsage / 1024 / 1024) << " MB\n";

    oss << "\nImage Operations:\n";
    oss << "  Images Decoded: " << currentMetrics_.imagesDecoded << "\n";
    oss << "  Avg Decode Time: " << std::setprecision(2) << currentMetrics_.averageDecodeTime << " ms\n";
    oss << "  Avg Upload Time: " << currentMetrics_.averageUploadTime << " ms\n";
    oss << "  Cache Hits: " << currentMetrics_.cacheHits << "\n";
    oss << "  Cache Misses: " << currentMetrics_.cacheMisses << "\n";

    oss << "\nGPU:\n";
    oss << "  Avg GPU Time: " << currentMetrics_.averageGPUTime << " ms\n";
    oss << "  Draw Calls: " << currentMetrics_.drawCalls << "\n";

    oss << "\n=== Custom Metrics ===\n";
    for (const auto& pair : stats_) {
        const auto& stats = pair.second;
        oss << "  " << stats->name << ":\n";
        oss << "    Count: " << stats->GetCount() << "\n";
        oss << "    Average: " << std::setprecision(3) << stats->GetAverage() << " ms\n";
        oss << "    Min: " << stats->GetMin() << " ms\n";
        oss << "    Max: " << stats->GetMax() << " ms\n";
    }

    oss << "\n=========================================\n";

    return oss.str();
}

// ScopedTimer implementation
ScopedTimer::ScopedTimer(PerformanceMonitor* monitor, const std::string& name)
    : monitor_(monitor)
    , name_(name)
{
    if (monitor_) {
        monitor_->StartTimer(name_);
    }
}

ScopedTimer::~ScopedTimer()
{
    if (monitor_) {
        monitor_->StopTimer(name_);
    }
}

// MemoryTracker implementation
size_t MemoryTracker::MemorySnapshot::GetTotal() const
{
    return workingSetSize;
}

MemoryTracker::MemorySnapshot MemoryTracker::GetSnapshot()
{
    MemorySnapshot snapshot;
    GetProcessMemoryInfo(snapshot);
    return snapshot;
}

size_t MemoryTracker::GetCurrentUsage()
{
    return GetSnapshot().workingSetSize;
}

size_t MemoryTracker::GetPeakUsage()
{
    return GetSnapshot().peakWorkingSetSize;
}

bool MemoryTracker::GetProcessMemoryInfo(MemorySnapshot& snapshot)
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(
        GetCurrentProcess(),
        (PROCESS_MEMORY_COUNTERS*)&pmc,
        sizeof(pmc)))
    {
        snapshot.workingSetSize = pmc.WorkingSetSize;
        snapshot.peakWorkingSetSize = pmc.PeakWorkingSetSize;
        snapshot.pagefileUsage = pmc.PagefileUsage;
        snapshot.privateBytes = pmc.PrivateUsage;
        return true;
    }

    return false;
}

// GPUMemoryTracker implementation
size_t GPUMemoryTracker::EstimateGPUMemory(
    size_t width,
    size_t height,
    size_t bytesPerPixel)
{
    // Base texture memory
    size_t baseMemory = width * height * bytesPerPixel;

    // Add overhead for mipmaps (approximately 1/3 more)
    size_t mipMemory = baseMemory / 3;

    // Add overhead for GPU resource management (~10%)
    size_t overhead = (baseMemory + mipMemory) / 10;

    return baseMemory + mipMemory + overhead;
}

size_t GPUMemoryTracker::EstimateMipMemory(
    size_t width,
    size_t height,
    size_t bytesPerPixel,
    size_t levels)
{
    size_t totalMemory = 0;

    for (size_t level = 0; level < levels; ++level) {
        size_t levelWidth = std::max(1ULL, width >> level);
        size_t levelHeight = std::max(1ULL, height >> level);
        totalMemory += levelWidth * levelHeight * bytesPerPixel;
    }

    return totalMemory;
}

} // namespace Utils
} // namespace UltraImageViewer
