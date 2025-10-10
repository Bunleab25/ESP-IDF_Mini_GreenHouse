# ESP-IDF v5.5 Installation Guide for Windows

## Method 1: ESP-IDF Installer (Recommended for beginners)

### Step 1: Download ESP-IDF Installer
1. Go to: https://dl.espressif.com/dl/esp-idf/
2. Download `esp-idf-tools-setup-online-5.5.0.exe` (or latest 5.5.x version)
3. Run the installer as Administrator

### Step 2: Installation Options
- Select "ESP-IDF v5.5.0" (or latest 5.5.x)
- Choose installation directory (default: `C:\Espressif\esp-idf`)
- Let it install Python, Git, and other dependencies
- This will take 15-30 minutes depending on your internet speed

### Step 3: Verify Installation
1. Open "ESP-IDF 5.5 PowerShell" from Start Menu
2. Navigate to your project: `cd "C:\Users\User\OneDrive\greenhouse_idf\greenhouse_idf"`
3. Run: `idf.py --version`

## Method 2: Manual Installation (Advanced users)

### Prerequisites
- Git for Windows: https://git-scm.com/download/win
- Python 3.8+: https://www.python.org/downloads/

### Step 1: Clone ESP-IDF
```powershell
# Create directory
mkdir C:\Espressif
cd C:\Espressif

# Clone ESP-IDF v5.5
git clone -b v5.5.0 --recursive https://github.com/espressif/esp-idf.git
```

### Step 2: Install Tools
```powershell
cd C:\Espressif\esp-idf
.\install.ps1 esp32
```

### Step 3: Set Environment (every time you want to use ESP-IDF)
```powershell
cd C:\Espressif\esp-idf
.\export.ps1
```

## Quick Start After Installation

1. **Open ESP-IDF PowerShell** (from Start Menu) or run export script
2. **Navigate to project:**
   ```powershell
   cd "C:\Users\User\OneDrive\greenhouse_idf\greenhouse_idf"
   ```
3. **Build project:**
   ```powershell
   idf.py build
   ```
4. **Flash to ESP32:**
   ```powershell
   idf.py -p COM3 flash  # Replace COM3 with your port
   ```
5. **Monitor output:**
   ```powershell
   idf.py -p COM3 monitor
   ```

## Troubleshooting

### Common Issues:
1. **"idf.py not found"**: ESP-IDF environment not activated
   - Solution: Use ESP-IDF PowerShell or run `.\export.ps1`

2. **Permission errors**: Run as Administrator
   - Right-click PowerShell → "Run as Administrator"

3. **Python version conflicts**: Ensure Python 3.8+ is installed
   - Check with: `python --version`

4. **Git not found**: Install Git for Windows
   - Download from: https://git-scm.com/download/win

### Finding COM Port:
- Open Device Manager → Ports (COM & LPT)
- Look for "Silicon Labs CP210x" or "CH340" or similar
- Note the COM port number (e.g., COM3, COM4)

## Next Steps After Installation

1. Open ESP-IDF PowerShell
2. Navigate to your project directory
3. Run our environment check: `.\check_env.ps1`
4. If successful, build the project: `idf.py build`

## Project Configuration

Before building, you may want to configure WiFi and other settings:
```powershell
idf.py menuconfig
```

This opens a configuration menu where you can set:
- WiFi credentials
- Serial console settings
- Component configurations