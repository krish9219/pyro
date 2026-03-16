# Pyro Programming Language — Windows Installer
# P.Y.R.O — Performance You Really Own
# Created by Aravind Pilla

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "  🔥 Pyro Installer for Windows" -ForegroundColor Yellow
Write-Host "  P.Y.R.O — Performance You Really Own" -ForegroundColor DarkYellow
Write-Host ""

$REPO = "krish9219/pyro"
$INSTALL_DIR = "$env:USERPROFILE\.pyro\bin"
$PYRO_DIR = "$env:USERPROFILE\.pyro"

# Create install directory
New-Item -ItemType Directory -Force -Path $INSTALL_DIR | Out-Null

# Check if git is available
$hasGit = Get-Command git -ErrorAction SilentlyContinue
if (-not $hasGit) {
    Write-Host "  Installing Git..." -ForegroundColor Cyan
    winget install Git.Git --accept-source-agreements --accept-package-agreements -h | Out-Null
    $env:PATH += ";C:\Program Files\Git\bin"
}

# Check if a C++ compiler is available
$hasCmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $hasCmake) {
    # Try common paths
    $cmakePaths = @(
        "C:\Program Files\CMake\bin",
        "C:\Program Files (x86)\CMake\bin",
        "${env:ProgramFiles}\CMake\bin"
    )
    foreach ($p in $cmakePaths) {
        if (Test-Path "$p\cmake.exe") {
            $env:PATH += ";$p"
            $hasCmake = $true
            break
        }
    }
}

if (-not $hasCmake) {
    Write-Host "  Installing CMake..." -ForegroundColor Cyan
    winget install Kitware.CMake --accept-source-agreements --accept-package-agreements -h | Out-Null
    $env:PATH += ";C:\Program Files\CMake\bin"
}

# Check for Visual Studio Build Tools
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$hasVS = $false
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
    if ($vsPath) { $hasVS = $true }
}

# Also check for g++ (MinGW)
$hasGpp = Get-Command g++ -ErrorAction SilentlyContinue

if (-not $hasVS -and -not $hasGpp) {
    Write-Host "  No C++ compiler found." -ForegroundColor Yellow
    Write-Host "  Installing Visual Studio Build Tools (this may take a few minutes)..." -ForegroundColor Cyan
    winget install Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --quiet" --accept-source-agreements --accept-package-agreements -h
    Write-Host "  ✓ Visual Studio Build Tools installed" -ForegroundColor Green
    Write-Host ""
    Write-Host "  IMPORTANT: Please restart PowerShell and run this script again." -ForegroundColor Yellow
    Write-Host "  The C++ compiler needs a fresh terminal to be detected." -ForegroundColor Yellow
    Write-Host ""
    exit 0
}

Write-Host "  Cloning Pyro..." -ForegroundColor Cyan
$buildDir = "$env:TEMP\pyro-build"
if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
git clone --depth 1 "https://github.com/$REPO.git" $buildDir 2>&1 | Out-Null

Write-Host "  Building Pyro..." -ForegroundColor Cyan
Push-Location $buildDir
New-Item -ItemType Directory -Force -Path build | Out-Null
Set-Location build

if ($hasVS) {
    # Use Visual Studio
    cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
    cmake --build . --config Release 2>&1 | Out-Null
    $exePath = "Release\pyro.exe"
} else {
    # Use MinGW
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
    cmake --build . 2>&1 | Out-Null
    $exePath = "pyro.exe"
}

if (Test-Path $exePath) {
    Copy-Item $exePath "$INSTALL_DIR\pyro.exe" -Force
    Write-Host "  ✓ Built successfully!" -ForegroundColor Green
} else {
    Write-Host "  Build failed. Check that Visual Studio Build Tools are installed." -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location
Remove-Item -Recurse -Force $buildDir -ErrorAction SilentlyContinue

# Add to PATH
$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($userPath -notlike "*\.pyro\bin*") {
    [Environment]::SetEnvironmentVariable("Path", "$userPath;$INSTALL_DIR", "User")
    $env:PATH += ";$INSTALL_DIR"
    Write-Host "  ✓ Added to PATH" -ForegroundColor Green
}

# Verify
Write-Host ""
& "$INSTALL_DIR\pyro.exe" version
Write-Host ""
Write-Host "  ✓ Pyro installed successfully!" -ForegroundColor Green
Write-Host "  Location: $INSTALL_DIR\pyro.exe" -ForegroundColor Gray
Write-Host ""
Write-Host "  Try it:" -ForegroundColor Cyan
Write-Host '  echo "print(""Hello from Pyro!"")" > hello.ro' -ForegroundColor White
Write-Host "  pyro run hello.ro" -ForegroundColor White
Write-Host ""
Write-Host "  Restart PowerShell for PATH to take effect." -ForegroundColor Yellow
Write-Host ""
