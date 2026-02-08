@echo off
setlocal

cd /d "E:\UltraImageViewer\winui\UltraImageViewer.WinUI"

echo ========================================
echo Building UltraImageViewer.WinUI
echo ========================================
echo.

"E:\VS\Common7\IDE\devenv.com" UltraImageViewer.WinUI.sln /build Release /out build_output.txt

type build_output.txt

echo.
echo ========================================

if exist "UltraImageViewer.WinUI\UltraImageViewer.WinUI.exe" (
    echo BUILD SUCCESS!
    echo.
    echo Output: UltraImageViewer.WinUI\UltraImageViewer.WinUI.exe
    echo.
) else (
    echo BUILD FAILED
    echo.
)

pause

endlocal
