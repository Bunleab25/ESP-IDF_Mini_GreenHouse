<#
activate_idf.ps1

Attempts to locate an ESP-IDF v5.x installation and activate (export) the environment
for the current PowerShell session so that `idf.py` becomes available.

Usage:
  powershell -ExecutionPolicy Bypass -File .\activate_idf.ps1
  # or inside an existing PS session:
  . .\activate_idf.ps1   (dot-source if you want the env to persist in current session)
#>

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " ESP-IDF Auto Activation Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

if ($env:IDF_PATH -and (Test-Path $env:IDF_PATH)) {
    Write-Host "Existing ESP-IDF environment detected: $env:IDF_PATH" -ForegroundColor Green
    if (Get-Command idf.py -ErrorAction SilentlyContinue) {
        Write-Host "idf.py already available. Nothing to do." -ForegroundColor Green
        return
    }
}

# Potential root locations (ordered by common installer defaults first)
$possibleRoots = @(
    # Official online/offline installer (user scope)
    "$env:USERPROFILE\esp",
    # Earlier / manual installs
    "C:\Espressif",
    # Newer installer nested frameworks directory
    "$env:USERPROFILE\.espressif\frameworks",
    "$env:USERPROFILE\.espressif",
    # Sometimes people clone directly here
    "$env:USERPROFILE\Documents",
    "$env:USERPROFILE\Desktop"
)

$exportScripts = @()
foreach ($root in $possibleRoots) {
    if (-not (Test-Path $root)) { continue }
    try {
        # PowerShell 5.1 does NOT support -Depth; we just use -Recurse with a size guard
        $found = Get-ChildItem -Path $root -Filter export.ps1 -File -Recurse -ErrorAction SilentlyContinue 2>$null |
            Where-Object { $_.FullName -match 'esp-idf' }
        if ($found) { $exportScripts += $found }
    } catch { }
}

# Heuristic: prefer paths that look like esp-idf-v5.* first, then plain esp-idf
$exportScripts = $exportScripts | Sort-Object {
    if ($_.FullName -match 'v5') { 0 } else { 1 }
}

if (-not $exportScripts -or $exportScripts.Count -eq 0) {
    Write-Host "❌ Could not locate any export.ps1 (ESP-IDF) scripts automatically." -ForegroundColor Red
    Write-Host ""; Write-Host "Manual options:" -ForegroundColor Yellow
    Write-Host "  1. Start Menu -> 'ESP-IDF 5.5 PowerShell' (preferred)" -ForegroundColor White
    Write-Host "  2. If you manually cloned: locate the folder containing 'tools' and run: . .\\export.ps1" -ForegroundColor White
    Write-Host "  3. Example common paths to try manually:" -ForegroundColor White
    Write-Host "     . C:\Espressif\esp-idf\export.ps1" -ForegroundColor Green
    Write-Host "     . $env:USERPROFILE\esp\esp-idf\export.ps1" -ForegroundColor Green
    Write-Host "     . $env:USERPROFILE\.espressif\frameworks\esp-idf-v5.5\export.ps1" -ForegroundColor Green
    Write-Host ""; Write-Host "If ESP-IDF isn't installed yet, run: .\\setup_esp_idf.bat" -ForegroundColor Yellow
    return
}

$chosen = $exportScripts[0]
$espIdfPath = Split-Path -Parent $chosen.FullName
Write-Host "Found candidate ESP-IDF at: $espIdfPath" -ForegroundColor Green
Write-Host "Activating..." -ForegroundColor Cyan

# Dot-source so env vars persist in current session if user dot-sources this script
. $chosen.FullName

if ($?) {
    if (Get-Command idf.py -ErrorAction SilentlyContinue) {
        $ver = idf.py --version 2>$null
        Write-Host "✅ Activated ESP-IDF ($ver)" -ForegroundColor Green
        Write-Host "IDF_PATH=$env:IDF_PATH" -ForegroundColor Green
        Write-Host ""; Write-Host "Next:" -ForegroundColor Cyan
        Write-Host "  idf.py build" -ForegroundColor White
        Write-Host "  idf.py -p COMx flash" -ForegroundColor White
        Write-Host "  idf.py -p COMx monitor" -ForegroundColor White
    } else {
        Write-Host "Activation ran, but idf.py still not found." -ForegroundColor Yellow
        Write-Host "You may need to DOT-SOURCE this script so env vars persist:" -ForegroundColor Yellow
        Write-Host "  . .\\activate_idf.ps1" -ForegroundColor Green
        Write-Host "(Note the space after the first dot.)" -ForegroundColor DarkYellow
    }
} else {
    Write-Host "❌ Failed to execute export script." -ForegroundColor Red
}