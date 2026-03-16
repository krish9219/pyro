# Pyro Programming Language - Complete Windows Installer
# P.Y.R.O - Performance You Really Own
# Created by Aravind Pilla
#
# Installs EVERYTHING:
#   1. Pyro compiler (pyro.exe)
#   2. MinGW C++ toolchain (g++, so 'pyro run' works)
#   3. Adds both to PATH
#
# No Visual Studio, no restarts, no admin needed.

$ErrorActionPreference = "Stop"
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$ProgressPreference = 'SilentlyContinue'

Write-Host ""
Write-Host "  ============================================" -ForegroundColor DarkYellow
Write-Host "       Pyro Installer for Windows" -ForegroundColor Yellow
Write-Host "    P.Y.R.O - Performance You Really Own" -ForegroundColor DarkYellow
Write-Host "  ============================================" -ForegroundColor DarkYellow
Write-Host ""

$REPO = "krish9219/pyro"
$PYRO_DIR = "$env:LOCALAPPDATA\Pyro"
$BIN_DIR = "$PYRO_DIR\bin"
$MINGW_DIR = "$PYRO_DIR\mingw"

# Create directories
New-Item -ItemType Directory -Force -Path $BIN_DIR | Out-Null

# ============================================
# STEP 1: Download Pyro compiler
# ============================================
Write-Host "  [1/3] Downloading Pyro compiler..." -ForegroundColor Cyan
$DEST = "$BIN_DIR\pyro.exe"

$downloaded = $false
$urls = @(
    "https://github.com/$REPO/releases/latest/download/pyro-windows-x86_64.exe",
    "https://aravindlabs.tech/pyro-lang/bin/pyro-windows-x86_64.exe"
)

foreach ($url in $urls) {
    try {
        Invoke-WebRequest -Uri $url -OutFile $DEST -UseBasicParsing -TimeoutSec 30
        if ((Get-Item $DEST).Length -gt 10000) {
            $downloaded = $true
            Write-Host "        Downloaded pyro.exe" -ForegroundColor Green
            break
        }
    } catch {
        continue
    }
}

if (-not $downloaded) {
    Write-Host ""
    Write-Host "  ERROR: Could not download Pyro." -ForegroundColor Red
    Write-Host "  Check your internet connection and try again." -ForegroundColor Yellow
    Write-Host "  Or download manually: https://github.com/$REPO/releases" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "  Press Enter to close"
    return
}

# ============================================
# STEP 2: Download MinGW C++ toolchain
# ============================================
Write-Host "  [2/3] Downloading MinGW C++ toolchain..." -ForegroundColor Cyan
Write-Host "        (needed for 'pyro run' - ~30 MB download)" -ForegroundColor Gray

$hasGpp = $false

# Check if g++ already exists
try {
    $null = Get-Command g++ -ErrorAction Stop
    $hasGpp = $true
    Write-Host "        g++ already installed - skipping!" -ForegroundColor Green
} catch {}

if (-not $hasGpp) {
    $mingwZip = "$env:TEMP\w64devkit.zip"
    $mingwDownloaded = $false

    # Try multiple w64devkit versions
    $mingwUrls = @(
        "https://github.com/skeeto/w64devkit/releases/download/v2.1.0/w64devkit-x64-2.1.0.zip",
        "https://github.com/skeeto/w64devkit/releases/download/v2.0.0/w64devkit-x64-2.0.0.zip",
        "https://github.com/skeeto/w64devkit/releases/download/v1.23.0/w64devkit-x64-1.23.0.zip"
    )

    foreach ($url in $mingwUrls) {
        try {
            Write-Host "        Downloading from GitHub..." -ForegroundColor Gray
            Invoke-WebRequest -Uri $url -OutFile $mingwZip -UseBasicParsing -TimeoutSec 120
            if ((Get-Item $mingwZip).Length -gt 1000000) {
                $mingwDownloaded = $true
                Write-Host "        Downloaded!" -ForegroundColor Green
                break
            }
        } catch {
            Write-Host "        Trying alternate mirror..." -ForegroundColor Gray
            continue
        }
    }

    if ($mingwDownloaded) {
        Write-Host "        Extracting (this takes ~1 minute)..." -ForegroundColor Gray

        # Remove old mingw if exists
        if (Test-Path $MINGW_DIR) {
            Remove-Item -Recurse -Force $MINGW_DIR -ErrorAction SilentlyContinue
        }

        # Extract
        try {
            Expand-Archive -Force -Path $mingwZip -DestinationPath $PYRO_DIR

            # w64devkit extracts into a 'w64devkit' subfolder - rename to 'mingw'
            if (Test-Path "$PYRO_DIR\w64devkit") {
                Rename-Item "$PYRO_DIR\w64devkit" "mingw"
            }

            if (Test-Path "$MINGW_DIR\bin\g++.exe") {
                $hasGpp = $true
                Write-Host "        MinGW g++ installed!" -ForegroundColor Green
            } else {
                Write-Host "        Extraction issue - g++ not found." -ForegroundColor Yellow
            }
        } catch {
            Write-Host "        Extraction failed: $_" -ForegroundColor Yellow
        }

        # Cleanup zip
        Remove-Item $mingwZip -Force -ErrorAction SilentlyContinue
    } else {
        Write-Host ""
        Write-Host "        MinGW download failed." -ForegroundColor Yellow
        Write-Host "        'pyro --version' and 'pyro check' will work fine." -ForegroundColor Gray
        Write-Host "        For 'pyro run', install a C++ compiler later:" -ForegroundColor Gray
        Write-Host "          winget install mingw" -ForegroundColor White
        Write-Host ""
    }
}

# ============================================
# STEP 3: Set up PATH
# ============================================
Write-Host "  [3/3] Setting up PATH..." -ForegroundColor Cyan

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
$pathsToAdd = @()

if ($userPath -notlike "*$BIN_DIR*") {
    $pathsToAdd += $BIN_DIR
}
if ($hasGpp -and (Test-Path "$MINGW_DIR\bin") -and ($userPath -notlike "*$MINGW_DIR*")) {
    $pathsToAdd += "$MINGW_DIR\bin"
}

if ($pathsToAdd.Count -gt 0) {
    $newPath = $userPath
    foreach ($p in $pathsToAdd) {
        $newPath = "$newPath;$p"
    }
    [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
    # Also update current session
    $env:PATH += ";" + ($pathsToAdd -join ";")
    Write-Host "        Added to PATH." -ForegroundColor Green
} else {
    Write-Host "        Already in PATH." -ForegroundColor Green
}

# ============================================
# DONE - Show results
# ============================================
Write-Host ""
Write-Host "  ============================================" -ForegroundColor Green
Write-Host "       Pyro installed successfully!" -ForegroundColor Green
Write-Host "  ============================================" -ForegroundColor Green
Write-Host ""

# Test pyro
Write-Host "  Verifying..." -ForegroundColor Gray
try {
    & "$BIN_DIR\pyro.exe" version
} catch {
    Write-Host "  Installed to: $BIN_DIR\pyro.exe" -ForegroundColor Gray
}

# Test g++
if ($hasGpp) {
    Write-Host "  g++ ready  -  'pyro run' will work!" -ForegroundColor Green
} else {
    Write-Host "  Note: g++ not available - 'pyro run' won't work yet." -ForegroundColor Yellow
    Write-Host "  Fix:  winget install mingw" -ForegroundColor White
}

Write-Host ""
Write-Host "  Installed to: $PYRO_DIR" -ForegroundColor Gray
Write-Host ""
Write-Host "  GET STARTED (open a NEW PowerShell window):" -ForegroundColor Cyan
Write-Host ""
Write-Host "    pyro --version" -ForegroundColor White
Write-Host '    echo "print(""Hello from Pyro!"")" > hello.ro' -ForegroundColor White
Write-Host "    pyro run hello.ro" -ForegroundColor White
Write-Host ""
Write-Host "  Docs: https://aravindlabs.tech/pyro-lang/docs.html" -ForegroundColor Gray
Write-Host ""
