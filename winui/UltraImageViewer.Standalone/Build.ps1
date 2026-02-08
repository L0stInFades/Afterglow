$ErrorActionPreference = "Continue"

$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptPath

Write-Host "========================================"
Write-Host "Building UltraImageViewer Standalone"
Write-Host "========================================"
Write-Host ""

# Setup environment
$vcTools = "E:\VS\VC\Tools\MSVC\14.50.35717"
$clPath = "$vcTools\bin\HostX64\x64\cl.exe"

Write-Host "Compiler: $clPath"
Write-Host ""

# Include paths
$include = @(
    "$vcTools\include",
    "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt",
    "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared",
    "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um"
) -join ';'

# Lib paths
$lib = @(
    "$vcTools\lib\x64",
    "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64",
    "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64"
) -join ';'

$env:INCLUDE = $include
$env:LIB = $lib

# Build arguments
$sourceFile = "UltraImageViewer.cpp"
$outputExe = "UltraImageViewer.exe"

$compileArgs = @(
    $sourceFile,
    "/EHsc",
    "/O2",
    "/DNDEBUG",
    "/Fe:$outputExe",
    "user32.lib",
    "gdiplus.lib",
    "shlwapi.lib",
    "shell32.lib",
    "ole32.lib",
    "/link",
    "/SUBSYSTEM:WINDOWS",
    "/MACHINE:X64"
)

Write-Host "Compiling..."
Write-Host ""

# Set VS environment first
$batch = @"
call "E:\VS\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
cl $sourceFile /EHsc /O2 /DNDEBUG /Fe:$outputExe user32.lib gdiplus.lib shlwapi.lib shell32.lib ole32.lib /link /SUBSYSTEM:WINDOWS /MACHINE:X64
exit %errorlevel%
"@

$batch | Out-File -FilePath "build_temp.bat" -Encoding ASCII

cmd.exe /c "build_temp.bat" 2>&1 | ForEach-Object { Write-Host $_ }
$exitCode = $LASTEXITCODE

Remove-Item "build_temp.bat" -ErrorAction SilentlyContinue

Write-Host ""
if (Test-Path $outputExe) {
    Write-Host "========================================"
    Write-Host "BUILD SUCCESS!"
    Write-Host "========================================"
    Write-Host ""
    Write-Host "Output: $(Resolve-Path $outputExe)"
    Write-Host "Size: $((Get-Item $outputExe).Length) bytes"
    Write-Host ""
    Write-Host "You can now run: .\$outputExe"
} else {
    Write-Host "========================================"
    Write-Host "BUILD FAILED"
    Write-Host "========================================"
}

Write-Host ""
