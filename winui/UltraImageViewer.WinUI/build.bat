@echo off
REM UltraImageViewer WinUI 3 Build Script
REM This script helps build the project using MSBuild

setlocal

echo ========================================
echo UltraImageViewer WinUI 3 Build Script
echo ========================================
echo.

REM Find MSBuild path (Visual Studio 2022)
set "MSBUILD="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)

if "%MSBUILD%"=="" (
    echo ERROR: Could not find MSBuild.exe
    echo Please install Visual Studio 2022 with "Desktop development with C++" workload
    pause
    exit /b 1
)

echo Found MSBuild at: %MSBUILD%
echo.

REM Check configuration
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

echo Building configuration: %CONFIG%
echo.

REM Restore NuGet packages
echo Restoring NuGet packages...
nuget restore UltraImageViewer.WinUI.sln
if errorlevel 1 (
    echo ERROR: NuGet restore failed
    echo Please install NuGet: https://www.nuget.org/downloads
    pause
    exit /b 1
)

echo.
echo Building solution...
echo.

REM Build the solution
"%MSBUILD%" UltraImageViewer.WinUI.sln /p:Configuration=%CONFIG% /p:Platform=x64 /v:minimal /nologo

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
echo Output location:
echo UltraImageViewer.WinUI\%CONFIG%\x64\
echo.
echo Press any key to run the application...
pause > nul

start UltraImageViewer.WinUI\%CONFIG%\x64\UltraImageViewer.WinUI.exe

endlocal
