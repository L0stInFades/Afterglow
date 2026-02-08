# UltraImageViewer - Architecture Design Document

## Overview
UltraImageViewer is an ultra-advanced, performance-friendly Windows image viewer designed for Windows 11.

## Core Principles
1. **Zero-Copy Architecture**: Minimize memory copies throughout the pipeline
2. **Hardware Acceleration**: Full GPU utilization via Direct2D/DirectX 12
3. **SIMD Optimization**: AVX2/AVX-512 for image processing
4. **Async I/O**: Non-blocking operations with event-driven design
5. **Smart Caching**: LRU cache with predictive pre-fetching

## System Architecture

### 1. Core Layer (`src/core/`)
- **ImageDecoder**: Zero-copy decoding pipeline
- **MemoryManager**: Memory-mapped file handling for large images
- **CacheManager**: LRU cache with size-based eviction
- **ThreadPool**: Work-stealing thread pool for async operations

### 2. Rendering Layer (`src/rendering/`)
- **D3D12Renderer**: DirectX 12 backend for GPU acceleration
- **Direct2DRenderer**: Direct2D + DirectComposition integration
- **TextureManager**: GPU texture management and upload
- **MipMapGenerator**: On-demand mipmap chain generation
- **Viewport**: Smart viewport culling for efficient rendering

### 3. UI Layer (`src/ui/`)
- **MainWindow**: Primary application window (Windows 11 style)
- **CommandPalette**: VSCode-style command palette (Ctrl+Shift+P)
- **GestureHandler**: Multi-touch gesture recognition
- **ThumbnailStrip**: Infinite scrolling thumbnail preview
- **MetadataPanel**: EXIF/GPS/metadata display

### 4. Image Processing Layer (`src/image_processing/`)
- **FilterPipeline**: Real-time filter application
- **ColorCorrection**: Auto white balance and color enhancement
- **SharpenEngine**: Smart adaptive sharpening
- **BatchProcessor**: Queue-based batch operations
- **RAWDecoder**: Hardware-accelerated RAW format support

### 5. Utilities (`src/utils/`)
- **PerformanceMonitor**: FPS, memory, timing instrumentation
- **Logger**: Structured logging system
- **ConfigManager**: JSON-based configuration

## Data Flow

```
[File I/O] -> [Memory Mapping] -> [Decoder] -> [Cache] -> [GPU Texture] -> [Renderer] -> [Display]
     |              |                |           |            |              |
     v              v                v           v            v              v
  Async          Zero-Copy        SIMD       LRU Cache    Direct2D/D3D12   Compositor
```

## Performance Targets

- **Startup Time**: < 500ms (cold start), < 100ms (warm start)
- **Image Loading**: < 100ms for 4K images (from cache)
- **Zoom/Pan**: 60 FPS minimum, 120 FPS target
- **Memory Usage**: < 500MB for typical workload
- **GPU Memory**: < 2GB for 8K images with full mipmaps

## Key Technologies

- **Graphics**: Direct2D 1.4, DirectComposition, DirectX 12
- **Image Decoding**: stb_image, libraw (optimized), WIC
- ** SIMD**: Intrinsics (AVX2, AVX-512), SSE4.2 fallback
- **Concurrency**: Windows Thread Pool, std::jthread
- **Build**: CMake 3.25+, Ninja/MSBuild

## Platform Requirements

- Windows 11 Build 22621+ (Direct2D 1.4 features)
- x64 architecture
- GPU: DirectX 12 Feature Level 11_0+
- RAM: 8GB minimum, 16GB recommended
- Storage: SSD recommended for large image handling

