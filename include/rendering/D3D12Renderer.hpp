#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <filesystem>
#include <functional>

namespace UltraImageViewer {
namespace Rendering {

using namespace Microsoft::WRL;

struct RenderTarget {
    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;
};

/**
 * DirectX 12 renderer for hardware-accelerated image display
 * Features:
 * - Multi-threaded command list recording
 * - Async GPU operations
 * - Descriptor heap management
 * - Resource batching
 */
class D3D12Renderer {
public:
    D3D12Renderer();
    ~D3D12Renderer();

    // Initialization
    bool Initialize(HWND hwnd, uint32_t width, uint32_t height);
    void Shutdown();

    // Render frame
    void Render(const std::vector<std::filesystem::path>& imagesToRender);
    void Present();

    // Resource management
    ComPtr<ID3D12Resource> CreateTexture(
        uint32_t width,
        uint32_t height,
        DXGI_FORMAT format,
        const void* initialData = nullptr
    );

    void UploadTexture(
        ID3D12Resource* texture,
        const void* data,
        size_t dataSize
    );

    // Viewport management
    void SetViewport(float x, float y, float width, float height);
    void SetScissorRect(LONG left, LONG top, LONG right, LONG bottom);

    // Resize handler
    void Resize(uint32_t newWidth, uint32_t newHeight);

    // Get device
    ID3D12Device* GetDevice() const { return device_.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }

    // Feature level query
    bool SupportsRaytracing() const;
    bool SupportsVariableRateShading() const;
    D3D12_FEATURE_DATA_D3D12_OPTIONS GetFeatures() const { return features_; }

private:
    // Device creation
    bool CreateDevice();
    bool CreateCommandObjects();
    bool CreateSwapChain(HWND hwnd);
    bool CreateDescriptorHeaps();
    bool CreateRenderTargetViews();

    // Synchronization
    bool CreateFences();
    void WaitForGPU();
    void MoveToNextFrame();

    // Pipeline state
    bool CreatePipelineStates();

    // Device and command objects
    ComPtr<ID3D12Device> device_;
    ComPtr<ID3D12CommandQueue> commandQueue_;
    ComPtr<ID3D12CommandAllocator> commandAllocator_;
    ComPtr<ID3D12GraphicsCommandList> commandList_;

    // Swap chain
    ComPtr<IDXGISwapChain4> swapChain_;
    static constexpr uint32_t FRAME_COUNT = 2;
    std::vector<RenderTarget> renderTargets_;
    uint32_t currentFrameIndex_ = 0;

    // Synchronization
    ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = NULL;

    // Descriptor heaps
    ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    ComPtr<ID3D12DescriptorHeap> dsvHeap_;
    ComPtr<ID3D12DescriptorHeap> cbvSrvUavHeap_;
    uint32_t rtvDescriptorSize_ = 0;

    // Window
    HWND hwnd_ = nullptr;
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    // Features
    D3D12_FEATURE_DATA_D3D12_OPTIONS features_;

    // Upload heap
    ComPtr<ID3D12Resource> uploadHeap_;
    size_t uploadHeapOffset_ = 0;
    static constexpr size_t UPLOAD_HEAP_SIZE = 256 * 1024 * 1024; // 256MB
};

/**
 * GPU texture uploader with async transfers
 */
class TextureUploader {
public:
    explicit TextureUploader(D3D12Renderer* renderer);
    ~TextureUploader();

    // Async upload
    void UploadAsync(
        ID3D12Resource* destination,
        const void* data,
        size_t dataSize,
        std::function<void()> completionCallback
    );

    // Wait for all uploads to complete
    void Flush();

private:
    D3D12Renderer* renderer_;
    ComPtr<ID3D12CommandQueue> uploadQueue_;
    ComPtr<ID3D12CommandAllocator> uploadAllocator_;
    ComPtr<ID3D12GraphicsCommandList> uploadList_;
    ComPtr<ID3D12Fence> uploadFence_;
    uint64_t uploadFenceValue_ = 0;
};

} // namespace Rendering
} // namespace UltraImageViewer
