@echo off
setlocal

cd /d "E:\UltraImageViewer\winui\UltraImageViewer.WinUI"

echo Building UltraImageViewer.WinUI.sln...
echo.

"E:\VS\Common7\IDE\devenv.com" UltraImageViewer.WinUI.sln /build Release > build_result.txt 2>&1

type build_result.txt

echo.
echo ========================================
echo Build completed. Exit code: %errorlevel%
echo ========================================

endlocal
