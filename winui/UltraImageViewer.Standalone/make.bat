@echo off
call "E:\VS\VC\Auxiliary\Build\vcvars64.bat"
cl UltraImageViewer.cpp /EHsc /O2 /Fe:UltraImageViewer.exe user32.lib gdi32.lib gdiplus.lib shlwapi.lib shell32.lib ole32.lib comdlg32.lib /link /SUBSYSTEM:WINDOWS
if exist UltraImageViewer.exe (
    echo.
    echo ========================================
    echo BUILD SUCCESS!
    echo ========================================
    dir UltraImageViewer.exe
) else (
    echo BUILD FAILED
)
pause
