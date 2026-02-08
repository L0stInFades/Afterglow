@echo off
setlocal

echo ========================================
echo Building UltraImageViewer Standalone
echo ========================================
echo.

cd /d "%~dp0"

echo Setting up Visual Studio environment...
call "E:\VS\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1

echo.
echo Compiling UltraImageViewer.cpp...
echo.

cl.exe UltraImageViewer.cpp /EHsc /O2 /DNDEBUG /Fe:UltraImageViewer.exe /link user32.lib gdiplus.lib shlwapi.lib shell32.lib ole32.lib /SUBSYSTEM:WINDOWS /MACHINE:X64

if errorlevel 1 (
    echo.
    echo ========================================
    echo BUILD FAILED
    echo ========================================
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESS!
echo ========================================
echo.
echo Output: UltraImageViewer.exe
echo.

dir UltraImageViewer.exe

pause

endlocal
