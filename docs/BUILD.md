# UltraImageViewer - Build Instructions

## Prerequisites

### Required Software

1. **Visual Studio 2022** (Community/Professional/Enterprise)
   - C++ Desktop Development workload
   - Windows 11 SDK (10.0.22621.0 or later)
   - MSVC v143 - VS 2022 C++ x64/x86 build tools

2. **CMake** 3.25 or later
   ```
   winget install Kitware.CMake
   # or download from: https://cmake.org/download/
   ```

3. **Git**
   ```
   winget install Git.Git
   # or download from: https://git-scm.com/download/win
   ```

4. **vcpkg** (automatically cloned by build script)
   - Or manually: `git clone https://github.com/Microsoft/vcpkg.git`
   - Run: `vcpkg\bootstrap-vcpkg.bat`

## Quick Start

### Automated Build (Recommended)

```batch
# Clone and navigate to project
cd UltraImageViewer

# Run build script (Release mode)
build.bat

# Debug build
build.bat debug

# Clean build
build.bat clean
build.bat
```

### Manual Build

```batch
# 1. Navigate to project directory
cd UltraImageViewer

# 2. Create and navigate to build directory
mkdir build
cd build

# 3. Configure with CMake
cmake -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DBUILD_TESTS=ON ^
    -DBUILD_BENCHMARKS=ON ^
    ..

# 4. Build
cmake --build . --config Release --parallel

# 5. Run
bin\Release\ultra_image_viewer.exe
```

## Build Configurations

| Configuration | Description | Optimization | Debug Info |
|--------------|-------------|--------------|------------|
| **Debug** | Development | None (/Od) | Full |
| **RelWithDebInfo** | Profiling | Speed (/O2) | Full |
| **Release** | Production | Full (/O2 /GL /LTCG) | None |

## Dependencies (Managed by vcpkg)

- **DirectX-Tex**: Texture processing
- **DirectX-Headers**: DirectX 12 headers
- **Windows Implementation Library (WIL)**: Windows helper utilities
- **STB**: Image decoding (stb_image)
- **libraw**: RAW format support
- **nlohmann-json**: Configuration
- **spdlog**: Logging
- **fmt**: Formatting

## Testing

```batch
# Build and run tests
build.bat

# Or manually:
cd build
ctest --config Release --output-on-failure
```

## Benchmarking

```batch
# Build and run benchmarks
cd build\benchmarks
Release\ultra_image_viewer_benchmarks.exe
```

## Installation

```batch
# Build installer
cd build
cpack -G WIX

# Or create portable ZIP
cpack -G ZIP
```

## Troubleshooting

### "vcpkg not found"
Ensure `vcpkg` directory exists or run build script which will clone it automatically.

### "CMake not found"
Add CMake to PATH or install via winget: `winget install Kitware.CMake`

### "Missing Windows SDK"
Install Windows 11 SDK from Visual Studio Installer or separately.

### "Link errors"
- Ensure you've run `vcpkg install` for all dependencies
- Check that `VCPKG_ROOT` environment variable is set correctly

### Build Performance Tips
- Use SSD for source code and build output
- Disable real-time antivirus scanning for build directory
- Use `--parallel` flag for parallel compilation
- Close unnecessary applications to free RAM

## IDE Setup

### Visual Studio 2022

1. Open folder: `File > Open > Folder...` and select `UltraImageViewer`
2. CMake will auto-configure (may take a minute)
3. Select build configuration from dropdown
4. Build: `Build > Build All` (Ctrl+Shift+B)

### VS Code

1. Install extensions:
   - C/C++ (ms-vscode.cpptools)
   - CMake Tools (ms-vscode.cmake-tools)

2. Open folder: `File > Open Folder`

3. Configure:
   - Press `Ctrl+Shift+P`
   - Select `CMake: Configure`
   - Choose kits: Visual Studio 2022 Release - x64

4. Build: `F7`

## Continuous Integration

See `.github/workflows/build.yml` for GitHub Actions configuration.

## Signing

To sign the executable with a code signing certificate:

```batch
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com ^
    bin\Release\ultra_image_viewer.exe
```
