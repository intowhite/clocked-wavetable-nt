# Windows PowerShell; uses the existing checked-out distingNT_API.
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$api = Join-Path $root "distingNT_API"
if ($env:NT_API_PATH) { $api = $env:NT_API_PATH }
$header = Join-Path $api "include\distingnt\wav.h"
if (!(Test-Path $header)) {
    Write-Host "The API was not found at $api" -ForegroundColor Yellow
    Write-Host "Run: git clone https://github.com/expertsleepersltd/distingNT_API.git"
    throw "Missing NT SDK checkout"
}
$compiler = Get-Command arm-none-eabi-g++ -ErrorAction SilentlyContinue
$nm = Get-Command arm-none-eabi-nm -ErrorAction SilentlyContinue
if (!$compiler -or !$nm) { throw "ARM compiler or arm-none-eabi-nm not found on PATH" }
$out = Join-Path $root "clocked_wavetable.o"
if (Test-Path $out) { Remove-Item $out -Force }
& $compiler.Source -std=c++11 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb `
    -fno-rtti -fno-exceptions -Os -fPIC -ffreestanding -fno-builtin `
    -fno-tree-loop-distribute-patterns -Wall -Wextra `
    "-I$(Join-Path $api 'include')" -c `
    "$(Join-Path $root 'clocked_wavetable.cpp')" -o $out
if ($LASTEXITCODE -ne 0) { throw "ARM compilation failed ($LASTEXITCODE)" }
$symbols = @(& $nm.Source --undefined-only $out)
if ($LASTEXITCODE -ne 0) { throw "Could not inspect ARM object" }
$unexpected = @()
foreach ($line in $symbols) {
    if ($line -match '\bU\s+(\S+)\s*$') {
        $symbol = $Matches[1]
        if ($symbol -notmatch '^(NT_|_GLOBAL_OFFSET_TABLE_$)') { $unexpected += $symbol }
    }
}
if ($unexpected.Count -gt 0) {
    Write-Host "WARNING: unresolved non-API runtime symbols:" -ForegroundColor Red
    $unexpected | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
    throw "Do NOT install this .o until these imports are resolved. Paste this list into ChatGPT."
}
Write-Host "PASS: no unexpected runtime imports" -ForegroundColor Green
Write-Host "Created $out" -ForegroundColor Green
Write-Host "Copy this .o to /programs/plug-ins/ on the NT SD card."
