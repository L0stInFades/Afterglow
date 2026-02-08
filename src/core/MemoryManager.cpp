#include "core/MemoryManager.hpp"
#include <stdexcept>
#include <algorithm>

namespace UltraImageViewer {
namespace Core {

// MemoryMappedFile implementation
MemoryMappedFile::MemoryMappedFile(const std::filesystem::path& filePath)
    : filePath_(filePath)
    , fileHandle_(INVALID_HANDLE_VALUE)
    , mappingHandle_(nullptr)
    , data_(nullptr)
    , size_(0)
{
}

MemoryMappedFile::~MemoryMappedFile()
{
    Unmap();
}

bool MemoryMappedFile::Map()
{
    if (data_ != nullptr) {
        return true; // Already mapped
    }

    // Open file
    fileHandle_ = CreateFileW(
        filePath_.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Get file size
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(fileHandle_, &fileSize)) {
        CloseHandle(fileHandle_);
        fileHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    size_ = static_cast<size_t>(fileSize.QuadPart);

    // Create file mapping
    mappingHandle_ = CreateFileMappingW(
        fileHandle_,
        nullptr,
        PAGE_READONLY,
        0,
        0,
        nullptr
    );

    if (mappingHandle_ == nullptr) {
        CloseHandle(fileHandle_);
        fileHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    // Map view of file
    data_ = static_cast<const uint8_t*>(MapViewOfFile(
        mappingHandle_,
        FILE_MAP_READ,
        0,
        0,
        0
    ));

    if (data_ == nullptr) {
        CloseHandle(mappingHandle_);
        CloseHandle(fileHandle_);
        mappingHandle_ = nullptr;
        fileHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

bool MemoryMappedFile::MapRegion(size_t offset, size_t size)
{
    // Unmap previous mapping
    Unmap();

    // Open file
    fileHandle_ = CreateFileW(
        filePath_.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Create file mapping
    mappingHandle_ = CreateFileMappingW(
        fileHandle_,
        nullptr,
        PAGE_READONLY,
        0,
        0,
        nullptr
    );

    if (mappingHandle_ == nullptr) {
        CloseHandle(fileHandle_);
        fileHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    // Map view of file with offset
    size_ = size;

    // Calculate aligned offset (allocation granularity is 64KB)
    const size_t allocationGranularity = 64 * 1024;
    size_t alignedOffset = (offset / allocationGranularity) * allocationGranularity;
    size_t offsetDelta = offset - alignedOffset;

    data_ = static_cast<const uint8_t*>(MapViewOfFile(
        mappingHandle_,
        FILE_MAP_READ,
        static_cast<DWORD>(alignedOffset >> 32),
        static_cast<DWORD>(alignedOffset & 0xFFFFFFFF),
        size + offsetDelta
    ));

    if (data_ == nullptr) {
        CloseHandle(mappingHandle_);
        CloseHandle(fileHandle_);
        mappingHandle_ = nullptr;
        fileHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }

    // Adjust data pointer to account for alignment
    data_ += offsetDelta;

    return true;
}

void MemoryMappedFile::Unmap()
{
    if (data_ != nullptr) {
        // Calculate original pointer if we adjusted for alignment
        const size_t allocationGranularity = 64 * 1024;
        uintptr_t addr = reinterpret_cast<uintptr_t>(data_);
        uintptr_t alignedAddr = addr & ~(allocationGranularity - 1);

        UnmapViewOfFile(reinterpret_cast<const void*>(alignedAddr));
        data_ = nullptr;
    }

    if (mappingHandle_ != nullptr) {
        CloseHandle(mappingHandle_);
        mappingHandle_ = nullptr;
    }

    if (fileHandle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(fileHandle_);
        fileHandle_ = INVALID_HANDLE_VALUE;
    }

    size_ = 0;
}

// ImageBufferPool implementation
ImageBufferPool::ImageBufferPool(size_t bufferSize, size_t maxPoolSize)
    : bufferSize_(bufferSize)
    , maxPoolSize_(maxPoolSize)
{
}

ImageBufferPool::~ImageBufferPool()
{
    Clear();
}

std::unique_ptr<uint8_t[]> ImageBufferPool::Allocate(size_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // Minimal safe allocation (pooling deferred)
    allocatedBuffers_++;
    totalBytesAllocated_ += size;
    return std::make_unique<uint8_t[]>(size);
}

void ImageBufferPool::Deallocate(std::unique_ptr<uint8_t[]> buffer, size_t size)
{
    // The custom deleter handles this
}

void ImageBufferPool::Clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    buffers_.clear();
    currentPoolSize_ = 0;
    allocatedBuffers_ = 0;
}

// GPUMemoryManager implementation
void* GPUMemoryManager::AllocateAligned(size_t size, size_t alignment)
{
    if (alignment == 0) {
        alignment = GPU_ALIGNMENT;
    }

    return _aligned_malloc(size, alignment);
}

void GPUMemoryManager::FreeAligned(void* ptr)
{
    _aligned_free(ptr);
}

bool GPUMemoryManager::IsAligned(const void* ptr, size_t alignment)
{
    if (alignment == 0) {
        alignment = GPU_ALIGNMENT;
    }

    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    return (addr & (alignment - 1)) == 0;
}

// VirtualMemoryManager implementation
void* VirtualMemoryManager::ReserveVirtual(size_t size)
{
    return VirtualAlloc(
        nullptr,
        size,
        MEM_RESERVE,
        PAGE_NOACCESS
    );
}

bool VirtualMemoryManager::CommitPages(void* ptr, size_t offset, size_t size)
{
    void* commitPtr = static_cast<uint8_t*>(ptr) + offset;

    void* result = VirtualAlloc(
        commitPtr,
        size,
        MEM_COMMIT,
        PAGE_READWRITE
    );

    return result != nullptr;
}

bool VirtualMemoryManager::DecommitPages(void* ptr, size_t offset, size_t size)
{
    void* decommitPtr = static_cast<uint8_t*>(ptr) + offset;

    return VirtualFree(
        decommitPtr,
        size,
        MEM_DECOMMIT
    ) != 0;
}

bool VirtualMemoryManager::ReleaseVirtual(void* ptr)
{
    return VirtualFree(
        ptr,
        0,
        MEM_RELEASE
    ) != 0;
}

void VirtualMemoryManager::TouchPages(void* ptr, size_t size)
{
    volatile uint8_t* data = static_cast<uint8_t*>(ptr);
    const size_t pageSize = 4096; // Windows page size

    for (size_t offset = 0; offset < size; offset += pageSize) {
        volatile uint8_t value = data[offset];
        (void)value; // Suppress unused warning
    }
}

} // namespace Core
} // namespace UltraImageViewer
