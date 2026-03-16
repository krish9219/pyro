# Pyro Programming Language - Windows Installer
# P.Y.R.O - Performance You Really Own
# Created by Aravind Pilla

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "  Pyro Installer for Windows" -ForegroundColor Yellow
Write-Host "  P.Y.R.O - Performance You Really Own" -ForegroundColor DarkYellow
Write-Host ""

$REPO = "krish9219/pyro"
$INSTALL_DIR = "$env:USERPROFILE\.pyro\bin"

# Create install directory
if (-not (Test-Path $INSTALL_DIR)) {
    New-Item -ItemType Directory -Force -Path $INSTALL_DIR | Out-Null
}

# Check if git is available
$hasGit = Get-Command git -ErrorAction SilentlyContinue
if (-not $hasGit) {
    Write-Host "  Installing Git..." -ForegroundColor Cyan
    winget install Git.Git --accept-source-agreements --accept-package-agreements -h
    $env:PATH += ";C:\Program Files\Git\bin"
}

# Check if cmake is available
$hasCmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $hasCmake) {
    $cmakePath = "C:\Program Files\CMake\bin"
    if (Test-Path "$cmakePath\cmake.exe") {
        $env:PATH += ";$cmakePath"
        $hasCmake = $true
    }
}
if (-not $hasCmake) {
    Write-Host "  Installing CMake..." -ForegroundColor Cyan
    winget install Kitware.CMake --accept-source-agreements --accept-package-agreements -h
    $env:PATH += ";C:\Program Files\CMake\bin"
}

# Check for Visual Studio Build Tools
$hasVS = $false
$vsWherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWherePath) {
    $vsPath = & $vsWherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
    if ($vsPath) { $hasVS = $true }
}

# Check for MinGW g++
$hasGpp = Get-Command g++ -ErrorAction SilentlyContinue

if (-not $hasVS -and -not $hasGpp) {
    Write-Host "  No C++ compiler found." -ForegroundColor Yellow
    Write-Host "  Installing Visual Studio Build Tools..." -ForegroundColor Cyan
    Write-Host "  This may take several minutes." -ForegroundColor Gray
    winget install Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --quiet" --accept-source-agreements --accept-package-agreements -h
    Write-Host ""
    Write-Host "  Visual Studio Build Tools installed." -ForegroundColor Green
    Write-Host "  Please RESTART PowerShell and run this script again." -ForegroundColor Yellow
    Write-Host ""
    Read-Host "  Press Enter to close"
    return
}

Write-Host "  Cloning Pyro..." -ForegroundColor Cyan
$buildDir = "$env:TEMP\pyro-build"
if (Test-Path $buildDir) {
    Remove-Item -Recurse -Force $buildDir
}
git clone --depth 1 "https://github.com/$REPO.git" $buildDir 2>&1 | Out-Null

Write-Host "  Building Pyro..." -ForegroundColor Cyan
Push-Location $buildDir
$bld = "$buildDir\build"
if (-not (Test-Path $bld)) {
    New-Item -ItemType Directory -Force -Path $bld | Out-Null
}
Set-Location $bld

$exePath = $null
if ($hasVS) {
    Write-Host "  Using Visual Studio compiler..." -ForegroundColor Gray
    cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
    cmake --build . --config Release 2>&1 | Out-Null
    $exePath = "$bld\Release\pyro.exe"
} elseif ($hasGpp) {
    Write-Host "  Using MinGW compiler..." -ForegroundColor Gray
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
    cmake --build . 2>&1 | Out-Null
    $exePath = "$bld\pyro.exe"
}

if ($exePath -and (Test-Path $exePath)) {
    Copy-Item $exePath "$INSTALL_DIR\pyro.exe" -Force
    Write-Host "  Built successfully!" -ForegroundColor Green
} else {
    Write-Host "  Build failed." -ForegroundColor Red
    Write-Host "  Make sure Visual Studio Build Tools with C++ workload are installed." -ForegroundColor Yellow
    Write-Host "  Then restart PowerShell and try again." -ForegroundColor Yellow
    Pop-Location
    Read-Host "  Press Enter to close"
    return
}

Pop-Location
Remove-Item -Recurse -Force $buildDir -ErrorAction SilentlyContinue

# Add to PATH
$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not ($userPath -like "*.pyro*")) {
    [Environment]::SetEnvironmentVariable("Path", "$userPath;$INSTALL_DIR", "User")
    $env:PATH += ";$INSTALL_DIR"
    Write-Host "  Added to PATH" -ForegroundColor Green
}

# Verify
Write-Host ""
try {
    & "$INSTALL_DIR\pyro.exe" version
} catch {
    Write-Host "  Warning: Could not verify installation" -ForegroundColor Yellow
}
Write-Host ""
Write-Host "  Pyro installed successfully!" -ForegroundColor Green
Write-Host "  Location: $INSTALL_DIR" -ForegroundColor Gray
Write-Host ""
Write-Host "  Try it:" -ForegroundColor Cyan
Write-Host "    pyro run hello.ro" -ForegroundColor White
Write-Host ""
Write-Host "  Restart PowerShell for PATH to take effect." -ForegroundColor Yellow
Write-Host ""
Read-Host "  Press Enter to close"
