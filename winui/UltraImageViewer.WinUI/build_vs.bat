@echo off
setlocal

echo Setting up Visual Studio environment...
call "E:\VS\VC\Auxiliary\Build\vcvars64.bat"

echo.
echo Building UltraImageViewer WinUI 3...
echo.

cd /d "%~dp0"

msbuild UltraImageViewer.WinUI.sln /p:Configuration=Release /p:Platform=x64 /v:normal /nologo

if errorlevel 1 (
    echo.
    echo ========================================
    echo BUILD FAILED
    echo ========================================
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESS
echo ========================================
echo.

pause

endlocal
