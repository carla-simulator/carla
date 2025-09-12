# PowerShell Environment Check Script for CARLA UE5.5 Build

Write-Host "`n===============================================" -ForegroundColor Green
Write-Host "CARLA UE5.5 Build Environment Check"
Write-Host "===============================================" -ForegroundColor Green

# 1. Check OS Version
Write-Host "`n1. Checking Operating System Version..." -ForegroundColor Cyan
Get-WmiObject Win32_OperatingSystem | Select-Object Caption, Version, BuildNumber

# 2. Check Visual Studio 2022 Installation
Write-Host "`n2. Checking Visual Studio 2022 Installation..." -ForegroundColor Cyan
$vsPaths = @(
    "D:\VS2022\Community",
    "C:\Program Files\Microsoft Visual Studio\2022\Community",
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community"
)

$vsFound = $false
foreach ($path in $vsPaths) {
    if (Test-Path "$path\Common7\IDE\devenv.exe") {
        Write-Host "[Found] Visual Studio 2022 at: $path"
        $vsFound = $true
        $vsInstallPath = $path
        break
    }
}

if (-not $vsFound) {
    Write-Host "[ERROR] Visual Studio 2022 NOT FOUND!" -ForegroundColor Red
}

# 3. Check for C++ Compiler (cl.exe)
Write-Host "`n3. Checking for C++ Compiler (cl.exe)..." -ForegroundColor Cyan
$clExe = Get-Command "cl.exe" -ErrorAction SilentlyContinue
if ($clExe) {
    Write-Host "[Found] C++ compiler at: $($clExe.Source)"
    # Try to get version info
    try {
        $clVersion = & "cl.exe" 2>&1 | Select-String "Version"
        Write-Host "[Version] $($clVersion.ToString().Trim())"
    } catch {
        Write-Host "[Info] Could not retrieve compiler version info."
    }
} else {
    Write-Host "[ERROR] C++ compiler (cl.exe) NOT FOUND!" -ForegroundColor Red
    Write-Host "[Solution] Open Visual Studio Installer and add the 'Desktop development with C++' workload." -ForegroundColor Yellow
    Write-Host "[Details] This is a critical component required for building CARLA."
}

# 4. Check CMake Installation
Write-Host "`n4. Checking CMake Installation..." -ForegroundColor Cyan
$cmakeExe = Get-Command "cmake.exe" -ErrorAction SilentlyContinue
if ($cmakeExe) {
    Write-Host "[Found] CMake at: $($cmakeExe.Source)"
    try {
        $cmakeVersion = & "cmake.exe" --version
        Write-Host "[Version] $($cmakeVersion.ToString().Trim())"
    } catch {
        Write-Host "[Info] Could not retrieve CMake version info."
    }
} else {
    Write-Host "[ERROR] CMake NOT FOUND!" -ForegroundColor Red
    Write-Host "[Solution] Install CMake 3.24 or newer." -ForegroundColor Yellow
}

# 5. Check Free Disk Space
Write-Host "`n5. Checking Available Disk Space (minimum 225GB required)..." -ForegroundColor Cyan
$systemDrive = Get-WmiObject Win32_LogicalDisk -Filter "DeviceID='C:'" | Select-Object FreeSpace, Size
$freeGB = [math]::Round($systemDrive.FreeSpace / 1GB, 2)
$totalGB = [math]::Round($systemDrive.Size / 1GB, 2)
Write-Host "[Drive C:] Free: $freeGB GB / Total: $totalGB GB"

if ($freeGB -lt 225) {
    Write-Host "[WARNING] Insufficient disk space! At least 225GB is recommended." -ForegroundColor Yellow
}

# 6. Check Windows SDK Installation
Write-Host "`n6. Checking Windows SDK Installation..." -ForegroundColor Cyan
$windowsSdkPath = "C:\Program Files (x86)\Windows Kits\10\Include"
if (Test-Path $windowsSdkPath) {
    Write-Host "[Found] Windows SDK directory at: $windowsSdkPath"
    Write-Host "[Versions] $(Get-ChildItem $windowsSdkPath -Directory | Select-Object -ExpandProperty Name)"
} else {
    Write-Host "[WARNING] Windows SDK directory not found at default location." -ForegroundColor Yellow
}

Write-Host "`n===============================================" -ForegroundColor Green
Write-Host "Environment Check Complete!"
Write-Host "===============================================" -ForegroundColor Green

if (-not $clExe) {
    Write-Host "`n[CRITICAL ISSUE] Missing C++ compiler (cl.exe) is preventing CARLA build." -ForegroundColor Red
    Write-Host "Please follow these steps to fix the issue:"
    Write-Host "1. Open Visual Studio Installer"
    Write-Host "2. Modify your Visual Studio 2022 installation"
    Write-Host "3. Select 'Desktop development with C++' workload"
    Write-Host "4. Ensure 'MSVC v143 - VS 2022 C++ x64/x86 build tools' is selected"
    Write-Host "5. Ensure 'Windows 11 SDK' or 'Windows 10 SDK' is selected"
    Write-Host "6. Click 'Modify' to install these components"
}