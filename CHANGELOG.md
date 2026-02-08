# Changelog

All notable changes to UltraImageViewer will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial project structure and architecture
- CMake build system with Release optimization
- Zero-copy image decoder design
- Memory-mapped file support architecture
- LRU cache with predictive pre-fetching
- SIMD function signatures (AVX2/AVX-512 with SSE4.2 fallback)
- Direct2D + DirectComposition integration
- DirectX 12 backend architecture
- Windows 11 Per-Monitor DPI Awareness V2
- Command palette design (VSCode-style)
- Multi-touch gesture support framework
- Image comparison architecture
- Batch processing queue design
- Metadata viewer framework
- Automated build script (build.bat)
- GitHub Actions CI/CD workflow

## [1.0.0] - TBD

### Planned Features
- Complete zero-copy decoder implementation
- Full SIMD image processing pipeline
- GPU-accelerated rendering
- Real-time filters
- RAW format support
- Batch operations
- Multi-monitor support
- Installer and portable versions

[Unreleased]: https://github.com/yourusername/UltraImageViewer/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/yourusername/UltraImageViewer/releases/tag/v1.0.0
