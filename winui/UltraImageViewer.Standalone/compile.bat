@echo off
setlocal EnableDelayedExpansion

cd /d "%~dp0"

echo Setting up build environment...

REM Setup VS environment
set "VCTools=E:\VS\VC\Tools\MSVC\14.50.35717"
set "VCToolsBin=%VCTools%\bin\HostX64\x64"
set "INCLUDE=E:\VS\VC\Tools\MSVC\14.50.35717\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\winrt"
set "LIB=E:\VS\VC\Tools\MSVC\14.50.35717\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64"

echo.
echo Compiler: %VCToolsBin%\cl.exe
echo.

REM Compile
"%VCToolsBin%\cl.exe" UltraImageViewer.cpp /EHsc /O2 /DNDEBUG /I"E:\VS\VC\Tools\MSVC\14.50.35717\include" /Fe:UltraImageViewer.exe /link user32.lib gdiplus.lib shlwapi.lib shell32.lib ole32.lib /SUBSYSTEM:WINDOWS /MACHINE:X64 /LIBPATH:"E:\VS\VC\Tools\MSVC\14.50.35717\lib\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64"

if exist UltraImageViewer.exe (
    echo.
    echo ========================================
    echo BUILD SUCCESS!
    echo ========================================
    echo.
    dir UltraImageViewer.exe
    echo.
    echo You can now run: UltraImageViewer.exe
) else (
    echo.
    echo ========================================
    echo BUILD FAILED
    echo ========================================
)

pause
endlocal
