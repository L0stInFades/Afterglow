@echo off
REM This script opens the solution in Visual Studio and starts a build

echo Opening UltraImageViewer.WinUI.sln in Visual Studio...
echo.

start devenv "E:\UltraImageViewer\winui\UltraImageViewer.WinUI\UltraImageViewer.WinUI.sln"

echo.
echo Visual Studio is opening...
echo.
echo Once VS opens, press Ctrl+Shift+B to build, or F5 to run.
echo.

timeout /t 3
