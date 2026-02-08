#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <mutex>
#include <Windows.h>
#include <wrl/client.h>

namespace UltraImageViewer {
namespace Core {

/**
 * Memory-mapped file manager for zero-copy large image handling
 * Uses Windows Memory-Mapped Files API for efficient access
 */
class MemoryMappedFile {
public:
    MemoryMappedFile(const std::filesystem::path& filePath);
    ~MemoryMappedFile();

    // Map entire file
    bool Map();

    // Map specific region
    bool MapRegion(size_t offset, size_t size);

    // Unmap
    void Unmap();

    // Access
    const uint8_t* GetData() const { return data_; }
    size_t GetSize() const { return size_; }
    bool IsMapped() const { return data_ != nullptr; }

    // File info
    const std::filesystem::path& GetPath() const { return filePath_; }

private:
    std::filesystem::path filePath_;
    HANDLE fileHandle_ = INVALID_HANDLE_VALUE;
    HANDLE mappingHandle_ = nullptr;
    const uint8_t* data_ = nullptr;
    size_t size_ = 0;
};

/**
 * Pool allocator for image buffers
 * Reduces allocation overhead and improves cache locality
 */
class ImageBufferPool {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
    static constexpr size_t MAX_POOL_SIZE = 512 * 1024 * 1024; // 512MB

    ImageBufferPool(size_t bufferSize = DEFAULT_BUFFER_SIZE, size_t maxPoolSize = MAX_POOL_SIZE);
    ~ImageBufferPool();

    // Allocate buffer from pool
    std::unique_ptr<uint8_t[]> Allocate(size_t size);

    // Return buffer to pool (optional, automatic on destruction)
    void Deallocate(std::unique_ptr<uint8_t[]> buffer, size_t size);

    // Pool statistics
    size_t GetPoolSize() const { return currentPoolSize_; }
    size_t GetAllocatedCount() const { return allocatedBuffers_; }
    size_t GetTotalBytesAllocated() const { return totalBytesAllocated_; }

    // Clear pool
    void Clear();

private:
    struct BufferSlot {
        std::unique_ptr<uint8_t[]> buffer;
        size_t size;
        bool inUse;
    };

    std::vector<BufferSlot> buffers_;
    size_t bufferSize_;
    size_t maxPoolSize_;
    size_t currentPoolSize_ = 0;
    size_t allocatedBuffers_ = 0;
    size_t totalBytesAllocated_ = 0;
    std::mutex mutex_;
};

/**
 * GPU-friendly memory manager
 * Ensures allocations are aligned for optimal DMA transfers
 */
class GPUMemoryManager {
public:
    static constexpr size_t GPU_ALIGNMENT = 256; // For AVX-512 and DMA
    static constexpr size_t GPU_PAGE_SIZE = 64 * 1024; // 64KB pages

    // Aligned allocation
    static void* AllocateAligned(size_t size, size_t alignment = GPU_ALIGNMENT);

    // Aligned deallocation
    static void FreeAligned(void* ptr);

    // Check if pointer is aligned
    static bool IsAligned(const void* ptr, size_t alignment = GPU_ALIGNMENT);

    // Round up to alignment
    static constexpr size_t AlignUp(size_t size, size_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
};

/**
 * Virtual memory manager for very large images
 * Uses MEM_RESERVE | MEM_COMMIT to reduce memory pressure
 */
class VirtualMemoryManager {
public:
    static constexpr size_t LARGE_ALLOCATION_THRESHOLD = 100 * 1024 * 1024; // 100MB

    // Reserve virtual address space
    static void* ReserveVirtual(size_t size);

    // Commit pages as needed
    static bool CommitPages(void* ptr, size_t offset, size_t size);

    // Decommit pages (free physical memory, keep virtual address)
    static bool DecommitPages(void* ptr, size_t offset, size_t size);

    // Release entire virtual allocation
    static bool ReleaseVirtual(void* ptr);

    // Touch pages to force commit
    static void TouchPages(void* ptr, size_t size);
};

} // namespace Core
} // namespace UltraImageViewer
