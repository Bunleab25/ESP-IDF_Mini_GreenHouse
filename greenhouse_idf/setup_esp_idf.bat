@echo off
echo ========================================
echo ESP-IDF v5.5 Quick Setup for Windows
echo ========================================
echo.
echo This script will help you set up ESP-IDF v5.5 for your greenhouse project.
echo.
echo OPTION 1: ESP-IDF Installer (Recommended)
echo ------------------------------------------
echo 1. Download ESP-IDF installer from:
echo    https://dl.espressif.com/dl/esp-idf/
echo 2. Look for "esp-idf-tools-setup-online-5.5.0.exe"
echo 3. Run as Administrator
echo 4. Select ESP-IDF v5.5.0
echo 5. After installation, use "ESP-IDF 5.5 PowerShell" from Start Menu
echo.
echo OPTION 2: Manual Installation
echo ------------------------------
echo 1. Install Git: https://git-scm.com/download/win
echo 2. Install Python 3.8+: https://www.python.org/downloads/
echo 3. Open PowerShell as Administrator and run:
echo    mkdir C:\Espressif
echo    cd C:\Espressif
echo    git clone -b v5.5.0 --recursive https://github.com/espressif/esp-idf.git
echo    cd esp-idf
echo    .\install.ps1 esp32
echo.
echo After Installation:
echo -------------------
echo 1. Open "ESP-IDF 5.5 PowerShell" from Start Menu (Option 1)
echo    OR run C:\Espressif\esp-idf\export.ps1 (Option 2)
echo 2. Navigate to: cd "%~dp0"
echo 3. Run: .\check_env.ps1
echo 4. If OK, run: idf.py build
echo.
echo ========================================
echo.
set /p choice="Would you like to open the ESP-IDF download page? (y/n): "
if /i "%choice%"=="y" (
    start https://dl.espressif.com/dl/esp-idf/
    echo Browser opened to ESP-IDF download page.
) else (
    echo Download ESP-IDF manually when ready.
)
echo.
echo See INSTALL_ESP_IDF.md for detailed instructions.
pause