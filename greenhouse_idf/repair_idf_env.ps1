<#
repair_idf_env.ps1
Repairs / (re)creates the missing ESP-IDF Python virtual environment and tools for a given esp-idf path.
Usage examples:
  powershell -ExecutionPolicy Bypass -File .\repair_idf_env.ps1 -IdfPath C:\Users\User\esp\v5.4.2\esp-idf -Targets esp32
  . .\repair_idf_env.ps1 -IdfPath C:\Users\User\esp\v5.4.2\esp-idf -Targets "esp32,esp32s3"
#>
param(
    [string]$IdfPath = "C:\Users\User\esp\v5.4.2\esp-idf",
    [string]$Targets = "esp32"
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " ESP-IDF Environment Repair" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Target esp-idf path: $IdfPath" -ForegroundColor Yellow

if (-not (Test-Path $IdfPath)) {
    Write-Host "❌ Provided path does not exist: $IdfPath" -ForegroundColor Red
    Write-Host "Edit the -IdfPath parameter or install ESP-IDF first." -ForegroundColor Yellow
    exit 1
}

$installScript = Join-Path $IdfPath 'install.ps1'
$exportScript  = Join-Path $IdfPath 'export.ps1'

if (-not (Test-Path $installScript)) { Write-Host "❌ install.ps1 not found in $IdfPath" -ForegroundColor Red; exit 1 }
if (-not (Test-Path $exportScript))  { Write-Host "❌ export.ps1 not found in $IdfPath" -ForegroundColor Red; exit 1 }

Write-Host "Running installer for targets: $Targets" -ForegroundColor Green
try {
    & $installScript $Targets
} catch {
    Write-Host "❌ install.ps1 failed: $_" -ForegroundColor Red
    exit 1
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ install.ps1 returned exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Running export.ps1" -ForegroundColor Green
try {
    . $exportScript
} catch {
    Write-Host "❌ export.ps1 failed: $_" -ForegroundColor Red
    exit 1
}

if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    Write-Host "⚠ export completed but idf.py still not available in this session." -ForegroundColor Yellow
    Write-Host "Try dot-sourcing this script instead: . .\\repair_idf_env.ps1" -ForegroundColor Yellow
} else {
    $ver = idf.py --version 2>$null
    Write-Host "✅ Repaired and activated ESP-IDF ($ver)" -ForegroundColor Green
    Write-Host "IDF_PATH=$env:IDF_PATH" -ForegroundColor Green
}

Write-Host "Done." -ForegroundColor Cyan