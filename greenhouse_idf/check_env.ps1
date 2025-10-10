# ESP-IDF Environment Check Script
# Run this script to verify ESP-IDF is properly set up

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP-IDF Environment Check" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

try {
    $idfPath = $env:IDF_PATH
    if (-not $idfPath) {
        Write-Host "❌ ERROR: IDF_PATH environment variable is not set!" -ForegroundColor Red
        Write-Host ""
        Write-Host "🔧 ESP-IDF is not installed or not activated!" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "📥 To install ESP-IDF v5.5:" -ForegroundColor Green
        Write-Host "   1. Run: .\setup_esp_idf.bat" -ForegroundColor White
        Write-Host "   2. Or see: INSTALL_ESP_IDF.md" -ForegroundColor White
        Write-Host ""
        Write-Host "🚀 Quick install (Recommended):" -ForegroundColor Green
        Write-Host "   1. Download ESP-IDF installer:" -ForegroundColor White
        Write-Host "      https://dl.espressif.com/dl/esp-idf/" -ForegroundColor Blue
        Write-Host "   2. Run 'esp-idf-tools-setup-online-5.5.0.exe' as Administrator" -ForegroundColor White
        Write-Host "   3. After install, open 'ESP-IDF 5.5 PowerShell' from Start Menu" -ForegroundColor White
        Write-Host "   4. Navigate back here and run this script again" -ForegroundColor White
        Write-Host ""
        Write-Host "🔧 Manual activation (if already installed):" -ForegroundColor Green
        Write-Host "   cd C:\Espressif\esp-idf" -ForegroundColor White
        Write-Host "   .\export.ps1" -ForegroundColor White
        Write-Host ""
        exit 1
    }
    
    Write-Host "✅ ESP-IDF Path: $idfPath" -ForegroundColor Green
    
    # Check if idf.py is available
    if (Get-Command idf.py -ErrorAction SilentlyContinue) {
        Write-Host "✅ idf.py is available" -ForegroundColor Green
        
        # Get ESP-IDF version
        $version = & idf.py --version 2>&1
        Write-Host "✅ ESP-IDF Version: $version" -ForegroundColor Green
        
        # Check Python version
        $pythonVersion = & python --version 2>&1
        Write-Host "✅ Python Version: $pythonVersion" -ForegroundColor Green
        
        Write-Host ""
        Write-Host "🎉 Environment is properly set up!" -ForegroundColor Green
        Write-Host ""
        Write-Host "📋 Next steps:" -ForegroundColor Cyan
        Write-Host "   1. Configure WiFi in main/main.c" -ForegroundColor White
        Write-Host "   2. Build project: idf.py build" -ForegroundColor White
        Write-Host "   3. Flash to ESP32: idf.py -p COMx flash" -ForegroundColor White
        Write-Host "   4. Monitor output: idf.py -p COMx monitor" -ForegroundColor White
        Write-Host ""
        Write-Host "💡 To find COM port: Check Device Manager > Ports" -ForegroundColor Yellow
        
    } else {
        Write-Host "❌ ERROR: idf.py command not found!" -ForegroundColor Red
        Write-Host ""
        Write-Host "🔧 Solutions:" -ForegroundColor Yellow
        Write-Host "   1. Open 'ESP-IDF 5.5 PowerShell' from Start Menu" -ForegroundColor White
        Write-Host "   2. Or run: C:\Espressif\esp-idf\export.ps1" -ForegroundColor White
        Write-Host "   3. If ESP-IDF not installed, run: .\setup_esp_idf.bat" -ForegroundColor White
        exit 1
    }
} catch {
    Write-Host "❌ Error checking environment: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "🔧 Try running this script from ESP-IDF PowerShell" -ForegroundColor Yellow
}