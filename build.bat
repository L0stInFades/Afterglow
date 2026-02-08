@echo off
REM UltraImageViewer Build Script
REM Requires: CMake 3.25+, vcpkg, Visual Studio 2022

setlocal EnableDelayedExpansion

echo ============================================
echo UltraImageViewer Build Script
echo ============================================
echo.

REM Check for CMake
where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] CMake not found in PATH
    echo Please install CMake 3.25 or later: https://cmake.org/download/
    exit /b 1
)

REM Check for vcpkg
if not exist "vcpkg\vcpkg.exe" (
    echo [INFO] vcpkg not found, cloning...
    git clone https://github.com/Microsoft/vcpkg.git
    if errorlevel 1 (
        echo [ERROR] Failed to clone vcpkg
        echo Please install Git: https://git-scm.com/download/win
        exit /b 1
    )
    call vcpkg\bootstrap-vcpkg.bat
)

REM Set build configuration
set BUILD_CONFIG=Release
set BUILD_DIR=build

REM Parse arguments
:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="debug" set BUILD_CONFIG=Debug
if /i "%~1"=="relwithdebinfo" set BUILD_CONFIG=RelWithDebInfo
if /i "%~1"=="clean" (
    echo [INFO] Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)
shift
goto parse_args
:end_parse

echo [INFO] Build Configuration: %BUILD_CONFIG%
echo.

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure with CMake
echo [INFO] Configuring project...
cd "%BUILD_DIR%"

cmake -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
    -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DBUILD_TESTS=ON ^
    -DBUILD_BENCHMARKS=ON ^
    ..

if errorlevel 1 (
    echo [ERROR] CMake configuration failed
    cd ..
    exit /b 1
)

cd ..

REM Build project
echo.
echo [INFO] Building project...
cmake --build "%BUILD_DIR%" --config %BUILD_CONFIG% --parallel

if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo.
echo ============================================
echo Build completed successfully!
echo ============================================
echo.
echo Output: %BUILD_DIR%\bin\%BUILD_CONFIG%\ultra_image_viewer.exe
echo.

REM Run tests if in Debug/RelWithDebInfo
if not "%BUILD_CONFIG%"=="Release" (
    echo [INFO] Running tests...
    ctest --test-dir "%BUILD_DIR%" --config %BUILD_CONFIG% --output-on-failure
)

endlocal
