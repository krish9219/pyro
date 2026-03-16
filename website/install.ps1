# Pyro Programming Language - Windows Installer
# P.Y.R.O - Performance You Really Own
# Created by Aravind Pilla
#
# ZERO dependencies. Just downloads the pre-built binary.
# No C++ compiler, no Visual Studio, no restarts needed.

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
# STEP 1: Download pre-built binary
# ============================================
Write-Host "  [1/2] Downloading Pyro..." -ForegroundColor Cyan

$BINARY_URL = "https://github.com/$REPO/releases/latest/download/pyro-windows-x86_64.exe"
$DEST = "$INSTALL_DIR\pyro.exe"

try {
    # Try downloading the pre-built binary
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -Uri $BINARY_URL -OutFile $DEST -UseBasicParsing
    Write-Host "        Downloaded successfully!" -ForegroundColor Green
} catch {
    Write-Host "        Pre-built binary not available yet. Trying direct download..." -ForegroundColor Yellow

    # Fallback: try the direct hosted binary
    $FALLBACK_URL = "https://aravindlabs.tech/pyro-lang/bin/pyro-windows-x86_64.exe"
    try {
        Invoke-WebRequest -Uri $FALLBACK_URL -OutFile $DEST -UseBasicParsing
        Write-Host "        Downloaded successfully!" -ForegroundColor Green
    } catch {
        Write-Host ""
        Write-Host "  Pre-built binary is not available yet." -ForegroundColor Red
        Write-Host "  Please download manually from:" -ForegroundColor Yellow
        Write-Host "    https://github.com/$REPO/releases" -ForegroundColor White
        Write-Host ""
        Write-Host "  Or build from source (requires C++ compiler):" -ForegroundColor Yellow
        Write-Host "    git clone https://github.com/$REPO.git" -ForegroundColor White
        Write-Host "    cd pyro && mkdir build && cd build" -ForegroundColor White
        Write-Host "    cmake .. && cmake --build . --config Release" -ForegroundColor White
        Write-Host ""
        Read-Host "  Press Enter to close"
        return
    }
}

# ============================================
# STEP 2: Add to PATH
# ============================================
Write-Host "  [2/2] Setting up PATH..." -ForegroundColor Cyan
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
