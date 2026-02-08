#pragma once

#include <memory>
#include <unordered_map>
#include <list>
#include <filesystem>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <wrl/client.h>
#include "ImageDecoder.hpp"

namespace UltraImageViewer {
namespace Core {

/**
 * LRU Cache with predictive pre-fetching
 * Thread-safe implementation with read-write locks
 */
class CacheManager {
public:
    struct CacheEntry {
        std::shared_ptr<DecodedImage> image;
        std::filesystem::path path;
        std::chrono::steady_clock::time_point lastAccess;
        size_t accessCount;
        size_t sizeBytes;

        // Pre-fetch metadata
        bool isPrefetched;
        std::chrono::steady_clock::time_point prefetchTime;
    };

    struct CacheStats {
        size_t hitCount = 0;
        size_t missCount = 0;
        size_t evictionCount = 0;
        size_t prefetchCount = 0;
        size_t currentSizeBytes = 0;
        size_t maxSizeBytes = 0;
        double hitRate = 0.0;
    };

    explicit CacheManager(size_t maxSizeBytes = 512 * 1024 * 1024); // 512MB default
    ~CacheManager();

    // Get image from cache
    std::shared_ptr<DecodedImage> Get(const std::filesystem::path& path);

    // Put image in cache
    void Put(const std::filesystem::path& path, std::shared_ptr<DecodedImage> image);

    // Pre-fetch image asynchronously
    void Prefetch(const std::filesystem::path& path);

    // Pre-fetch multiple images (batch)
    void PrefetchBatch(const std::vector<std::filesystem::path>& paths);

    // Remove specific entry
    void Remove(const std::filesystem::path& path);

    // Clear entire cache
    void Clear();

    // Get cache statistics
    CacheStats GetStats() const;

    // Resize cache
    void Resize(size_t newSizeBytes);

    // Enable/disable pre-fetching
    void SetPrefetchEnabled(bool enabled) { prefetchEnabled_ = enabled; }

    // Access pattern prediction
    void RecordAccess(const std::filesystem::path& path);

private:
    // LRU eviction
    void EvictIfNeeded();

    // Predict next accesses based on history
    std::vector<std::filesystem::path> PredictNextAccesses(
        const std::filesystem::path& currentPath
    );

    // Calculate entry size
    size_t CalculateEntrySize(const DecodedImage& image);

    // Thread safety
    mutable std::shared_mutex mutex_;

    // Cache storage
    std::unordered_map<std::filesystem::path, std::list<CacheEntry>::iterator> cacheIndex_;
    std::list<CacheEntry> lruList_;

    // Configuration
    size_t maxSizeBytes_;
    size_t currentSizeBytes_ = 0;

    // Statistics
    std::atomic<size_t> hitCount_{0};
    std::atomic<size_t> missCount_{0};
    std::atomic<size_t> evictionCount_{0};
    std::atomic<size_t> prefetchCount_{0};

    // Pre-fetching
    bool prefetchEnabled_ = true;
    std::unordered_map<std::filesystem::path, size_t> accessHistory_;

    // Background pre-fetch thread
    std::jthread prefetchThread_;
    std::condition_variable_any prefetchCV_;
    std::queue<std::filesystem::path> prefetchQueue_;
};

/**
 * Texture cache for GPU resources
 * Manages D3D12/Direct2D texture objects
 */
class TextureCache {
public:
    struct TextureEntry {
        Microsoft::WRL::ComPtr<IUnknown> texture; // ID3D12Resource or ID2D1Bitmap
        size_t sizeBytes;
        std::chrono::steady_clock::time_point lastUsed;
    };

    explicit TextureCache(size_t maxSizeBytes = 2ULL * 1024 * 1024 * 1024); // 2GB for GPU
    ~TextureCache();

    // Get texture from cache
    template<typename T>
    Microsoft::WRL::ComPtr<T> Get(const std::filesystem::path& path);

    // Put texture in cache
    void Put(const std::filesystem::path& path, IUnknown* texture, size_t sizeBytes);

    // Clear cache
    void Clear();

    // Get cache size
    size_t GetCurrentSize() const { return currentSizeBytes_; }

private:
    std::unordered_map<std::filesystem::path, TextureEntry> cache_;
    size_t maxSizeBytes_;
    size_t currentSizeBytes_ = 0;
    mutable std::mutex mutex_;
};

} // namespace Core
} // namespace UltraImageViewer
