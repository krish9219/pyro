# Pyro Programming Language - Windows Installer
# P.Y.R.O - Performance You Really Own
# Created by Aravind Pilla
#
# This script handles EVERYTHING automatically:
# 1. Installs Git (if missing)
# 2. Installs CMake (if missing)
# 3. Installs Visual Studio Build Tools + C++ workload (if missing)
# 4. Clones, builds, and installs Pyro
# 5. Adds to PATH

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "  ========================================" -ForegroundColor DarkYellow
Write-Host "    Pyro Installer for Windows" -ForegroundColor Yellow
Write-Host "    P.Y.R.O - Performance You Really Own" -ForegroundColor DarkYellow
Write-Host "  ========================================" -ForegroundColor DarkYellow
Write-Host ""

$REPO = "krish9219/pyro"
$INSTALL_DIR = "$env:USERPROFILE\.pyro\bin"

# Create install directory
if (-not (Test-Path $INSTALL_DIR)) {
    New-Item -ItemType Directory -Force -Path $INSTALL_DIR | Out-Null
}

# ============================================
# STEP 1: Check and install Git
# ============================================
Write-Host "  [1/5] Checking Git..." -ForegroundColor Cyan
$hasGit = Get-Command git -ErrorAction SilentlyContinue
if ($hasGit) {
    Write-Host "        Found: $(git --version)" -ForegroundColor Green
} else {
    Write-Host "        Installing Git..." -ForegroundColor Yellow
    winget install Git.Git --accept-source-agreements --accept-package-agreements -h
    $env:PATH += ";C:\Program Files\Git\bin"
    Write-Host "        Git installed." -ForegroundColor Green
}

# ============================================
# STEP 2: Check and install CMake
# ============================================
Write-Host "  [2/5] Checking CMake..." -ForegroundColor Cyan
$hasCmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $hasCmake) {
    # Check common install location
    if (Test-Path "C:\Program Files\CMake\bin\cmake.exe") {
        $env:PATH += ";C:\Program Files\CMake\bin"
        $hasCmake = $true
    }
}
if ($hasCmake) {
    Write-Host "        Found: $(cmake --version | Select-Object -First 1)" -ForegroundColor Green
} else {
    Write-Host "        Installing CMake..." -ForegroundColor Yellow
    winget install Kitware.CMake --accept-source-agreements --accept-package-agreements -h
    $env:PATH += ";C:\Program Files\CMake\bin"
    Write-Host "        CMake installed." -ForegroundColor Green
}

# ============================================
# STEP 3: Check and install C++ compiler
# ============================================
Write-Host "  [3/5] Checking C++ compiler..." -ForegroundColor Cyan

# Find vswhere
$hasVS = $false
$vsWherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWherePath) {
    $vsPath = & $vsWherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
    if ($vsPath) {
        $hasVS = $true
        Write-Host "        Found: Visual Studio C++ at $vsPath" -ForegroundColor Green
    }
}

# Check MinGW as alternative
$hasGpp = Get-Command g++ -ErrorAction SilentlyContinue
if ($hasGpp) {
    Write-Host "        Found: $(g++ --version | Select-Object -First 1)" -ForegroundColor Green
}

if (-not $hasVS -and -not $hasGpp) {
    Write-Host "        No C++ compiler found. Installing..." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "        This installs Visual Studio Build Tools with C++ support." -ForegroundColor Gray
    Write-Host "        It may take 5-10 minutes. Please be patient." -ForegroundColor Gray
    Write-Host ""

    # First install the base Build Tools
    $setupExe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\setup.exe"
    if (Test-Path $setupExe) {
        # Build Tools installer exists, just add C++ workload
        Write-Host "        Adding C++ workload to existing Build Tools..." -ForegroundColor Yellow
        $installPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022\BuildTools"
        Start-Process $setupExe -ArgumentList "modify --installPath `"$installPath`" --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --quiet --wait" -Wait
    } else {
        # No installer at all, use winget
        Write-Host "        Installing Visual Studio Build Tools + C++..." -ForegroundColor Yellow
        winget install Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --quiet --wait" --accept-source-agreements --accept-package-agreements
    }

    # Verify it worked
    Start-Sleep -Seconds 3
    if (Test-Path $vsWherePath) {
        $vsPath = & $vsWherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
        if ($vsPath) {
            $hasVS = $true
            Write-Host "        C++ compiler installed successfully!" -ForegroundColor Green
        }
    }

    if (-not $hasVS) {
        Write-Host ""
        Write-Host "        C++ compiler installation may need a restart." -ForegroundColor Yellow
        Write-Host "        Please:" -ForegroundColor Yellow
        Write-Host "          1. Close this window" -ForegroundColor White
        Write-Host "          2. Restart your computer" -ForegroundColor White
        Write-Host "          3. Run this installer again" -ForegroundColor White
        Write-Host ""
        Read-Host "  Press Enter to close"
        return
    }
}

# ============================================
# STEP 4: Clone and Build Pyro
# ============================================
Write-Host "  [4/5] Building Pyro..." -ForegroundColor Cyan

$buildDir = "$env:TEMP\pyro-build"
if (Test-Path $buildDir) {
    Remove-Item -Recurse -Force $buildDir
}

Write-Host "        Cloning repository..." -ForegroundColor Gray
git clone --depth 1 "https://github.com/$REPO.git" $buildDir 2>&1 | Out-Null

Push-Location $buildDir
$bld = "$buildDir\build"
New-Item -ItemType Directory -Force -Path $bld | Out-Null
Set-Location $bld

$exePath = $null
if ($hasVS) {
    # Find vcvarsall.bat to set up the compiler environment
    $vcvarsPath = & $vsWherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "VC\Auxiliary\Build\vcvarsall.bat" 2>$null
    if ($vcvarsPath) {
        Write-Host "        Configuring Visual Studio environment..." -ForegroundColor Gray
        # Use cmd to run vcvarsall and then cmake
        $cmdScript = "@echo off`r`ncall `"$vcvarsPath`" x64 >nul 2>&1`r`ncmake .. -DCMAKE_BUILD_TYPE=Release >nul 2>&1`r`ncmake --build . --config Release"
        $cmdFile = "$bld\build_pyro.bat"
        Set-Content -Path $cmdFile -Value $cmdScript
        Write-Host "        Compiling (this takes 1-2 minutes)..." -ForegroundColor Gray
        $process = Start-Process "cmd.exe" -ArgumentList "/c $cmdFile" -WorkingDirectory $bld -Wait -NoNewWindow -PassThru
        $exePath = "$bld\Release\pyro.exe"
        if (-not (Test-Path $exePath)) {
            $exePath = "$bld\pyro.exe"
        }
    } else {
        # Fallback: try cmake directly with VS generator
        Write-Host "        Configuring with CMake..." -ForegroundColor Gray
        cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
        Write-Host "        Compiling (this takes 1-2 minutes)..." -ForegroundColor Gray
        cmake --build . --config Release 2>&1 | Out-Null
        $exePath = "$bld\Release\pyro.exe"
    }
} elseif ($hasGpp) {
    Write-Host "        Using MinGW..." -ForegroundColor Gray
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
    cmake --build . 2>&1 | Out-Null
    $exePath = "$bld\pyro.exe"
}

if ($exePath -and (Test-Path $exePath)) {
    Copy-Item $exePath "$INSTALL_DIR\pyro.exe" -Force
    Write-Host "        Build successful!" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "  Build failed." -ForegroundColor Red
    Write-Host "  Try using 'Developer Command Prompt for VS 2022' instead:" -ForegroundColor Yellow
    Write-Host "    cd $env:USERPROFILE\pyro\build" -ForegroundColor White
    Write-Host "    cmake .." -ForegroundColor White
    Write-Host "    cmake --build . --config Release" -ForegroundColor White
    Write-Host ""
    Pop-Location
    Read-Host "  Press Enter to close"
    return
}

Pop-Location
Remove-Item -Recurse -Force $buildDir -ErrorAction SilentlyContinue

# ============================================
# STEP 5: Add to PATH
# ============================================
Write-Host "  [5/5] Setting up PATH..." -ForegroundColor Cyan
$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not ($userPath -like "*.pyro*")) {
    [Environment]::SetEnvironmentVariable("Path", "$userPath;$INSTALL_DIR", "User")
    $env:PATH += ";$INSTALL_DIR"
    Write-Host "        Added to PATH." -ForegroundColor Green
} else {
    Write-Host "        Already in PATH." -ForegroundColor Green
}

# ============================================
# DONE
# ============================================
Write-Host ""
Write-Host "  ========================================" -ForegroundColor Green
Write-Host "    Pyro installed successfully!" -ForegroundColor Green
Write-Host "  ========================================" -ForegroundColor Green
Write-Host ""
try {
    & "$INSTALL_DIR\pyro.exe" version
} catch {
    Write-Host "  Installed to: $INSTALL_DIR\pyro.exe" -ForegroundColor Gray
}
Write-Host ""
Write-Host "  Quick start:" -ForegroundColor Cyan
Write-Host '    echo "print(""Hello from Pyro!"")" > hello.ro' -ForegroundColor White
Write-Host "    pyro run hello.ro" -ForegroundColor White
Write-Host ""
Write-Host "  Restart PowerShell for PATH changes." -ForegroundColor Yellow
Write-Host ""
Read-Host "  Press Enter to close"
