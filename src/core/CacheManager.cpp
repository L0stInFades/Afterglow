#include "core/CacheManager.hpp"

namespace UltraImageViewer {
namespace Core {

CacheManager::CacheManager(size_t maxSizeBytes)
    : maxSizeBytes_(maxSizeBytes)
{
}

CacheManager::~CacheManager() = default;

std::shared_ptr<DecodedImage> CacheManager::Get(const std::filesystem::path& path)
{
    std::unique_lock lock(mutex_);

    auto it = cacheIndex_.find(path);
    if (it == cacheIndex_.end()) {
        missCount_++;
        return nullptr;
    }

    hitCount_++;

    // Move to front of LRU list
    lruList_.splice(lruList_.begin(), lruList_, it->second);
    it->second = lruList_.begin();
    it->second->lastAccess = std::chrono::steady_clock::now();
    it->second->accessCount++;

    return it->second->image;
}

void CacheManager::Put(const std::filesystem::path& path, std::shared_ptr<DecodedImage> image)
{
    std::unique_lock lock(mutex_);

    // Check if already in cache
    auto existing = cacheIndex_.find(path);
    if (existing != cacheIndex_.end()) {
        return;
    }

    size_t entrySize = CalculateEntrySize(*image);

    // Check if we need to evict
    while (currentSizeBytes_ + entrySize > maxSizeBytes_ && !lruList_.empty()) {
        EvictIfNeeded();
    }

    // Add new entry
    CacheEntry entry;
    entry.image = std::move(image);
    entry.path = path;
    entry.lastAccess = std::chrono::steady_clock::now();
    entry.accessCount = 1;
    entry.sizeBytes = entrySize;
    entry.isPrefetched = false;

    lruList_.push_front(std::move(entry));
    cacheIndex_[path] = lruList_.begin();

    currentSizeBytes_ += entrySize;
}

void CacheManager::Prefetch(const std::filesystem::path& path)
{
    // TODO: Implement async pre-fetching
}

void CacheManager::PrefetchBatch(const std::vector<std::filesystem::path>& paths)
{
    // TODO: Implement batch pre-fetching
}

void CacheManager::Remove(const std::filesystem::path& path)
{
    std::unique_lock lock(mutex_);

    auto it = cacheIndex_.find(path);
    if (it != cacheIndex_.end()) {
        currentSizeBytes_ -= it->second->sizeBytes;
        lruList_.erase(it->second);
        cacheIndex_.erase(it);
    }
}

void CacheManager::Clear()
{
    std::unique_lock lock(mutex_);
    lruList_.clear();
    cacheIndex_.clear();
    currentSizeBytes_ = 0;
}

CacheManager::CacheStats CacheManager::GetStats() const
{
    std::shared_lock lock(mutex_);

    CacheStats stats;
    stats.hitCount = hitCount_;
    stats.missCount = missCount_;
    stats.evictionCount = evictionCount_;
    stats.prefetchCount = prefetchCount_;
    stats.currentSizeBytes = currentSizeBytes_;
    stats.maxSizeBytes = maxSizeBytes_;

    size_t total = hitCount_ + missCount_;
    if (total > 0) {
        stats.hitRate = static_cast<double>(hitCount_) / total;
    }

    return stats;
}

void CacheManager::Resize(size_t newSizeBytes)
{
    std::unique_lock lock(mutex_);
    maxSizeBytes_ = newSizeBytes;
    EvictIfNeeded();
}

void CacheManager::RecordAccess(const std::filesystem::path& path)
{
    std::unique_lock lock(mutex_);
    accessHistory_[path]++;
}

void CacheManager::EvictIfNeeded()
{
    if (lruList_.empty()) {
        return;
    }

    auto& entry = lruList_.back();
    currentSizeBytes_ -= entry.sizeBytes;
    cacheIndex_.erase(entry.path);
    lruList_.pop_back();
    evictionCount_++;
}

std::vector<std::filesystem::path> CacheManager::PredictNextAccesses(
    const std::filesystem::path& currentPath)
{
    // TODO: Implement access pattern prediction
    return {};
}

size_t CacheManager::CalculateEntrySize(const DecodedImage& image)
{
    return image.info.dataSize;
}

} // namespace Core
} // namespace UltraImageViewer
