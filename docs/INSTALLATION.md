# Pyro Installation Guide

> Pyro Programming Language — Created by **Aravind Pilla**
>
> *Install once, code everywhere.*

Pyro runs on **Windows, macOS, and Linux**. Since Pyro transpiles to C++20, you need a C++20-compatible compiler. Pyro ships with **76 built-in modules** -- no package manager needed. Source files use the `.ro` extension.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Install](#quick-install)
- [Linux](#linux)
- [macOS](#macos)
- [Windows](#windows)
- [Building from Source](#building-from-source)
- [Package Managers](#package-managers)
- [IDE Extensions and Editor Support](#ide-extensions-and-editor-support)
- [PATH Setup](#path-setup)
- [Docker](#docker)
- [Verifying Your Installation](#verifying-your-installation)
- [Pyro Commands](#pyro-commands)
- [Updating Pyro](#updating-pyro)
- [Uninstalling](#uninstalling)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

Pyro requires a **C++20-compatible compiler** and CMake to build. C++20 is required because Pyro's transpiled output uses `auto` parameters and other C++20 features for fully dynamic typing.

| Platform | Compiler | Minimum Version |
|----------|----------|-----------------|
| Linux | GCC | 10.0+ |
| Linux | Clang | 10.0+ |
| macOS | Apple Clang (Xcode) | 13.0+ |
| macOS | GCC (Homebrew) | 10.0+ |
| Windows | MSVC (Visual Studio) | 2019+ (v16.10+) |
| Windows | MinGW-w64 | 10.0+ |

**Also required:**
- CMake 3.16 or newer
- Git (for cloning the repository)

---

## Quick Install

### Linux / macOS (One-Line Install)

```bash
curl -fsSL https://aravindlabs.tech/pyro-lang/install.sh | bash
```

Downloads a pre-built binary to `~/.pyro/bin/` and adds it to your PATH.

### Windows (PowerShell)

```powershell
irm https://aravindlabs.tech/pyro-lang/install.ps1 | iex
```

This downloads:
- `pyro.exe` — the Pyro compiler (pre-built, statically linked)
- MinGW g++ toolchain (~79 MB) — so `pyro run` and `pyro build` work immediately

No Visual Studio, no restarts, no admin needed. Installs to `%LOCALAPPDATA%\Pyro\`.

---

## Linux

### Ubuntu / Debian

```bash
# Install prerequisites (need g++ 10+ for C++20)
sudo apt update
sudo apt install -y g++-10 cmake git build-essential

# Clone and install Pyro
git clone https://github.com/krish9219/pyro.git
cd pyro
chmod +x install.sh
./install.sh
```

### Fedora / RHEL / CentOS

```bash
# Install prerequisites
sudo dnf install -y gcc-c++ cmake git make

# Clone and install Pyro
git clone https://github.com/krish9219/pyro.git
cd pyro
chmod +x install.sh
./install.sh
```

### Arch Linux / Manjaro

```bash
# Install prerequisites
sudo pacman -S --needed gcc cmake git base-devel

# Clone and install Pyro
git clone https://github.com/krish9219/pyro.git
cd pyro
chmod +x install.sh
./install.sh
```

### openSUSE

```bash
# Install prerequisites
sudo zypper install -y gcc-c++ cmake git make

# Clone and install Pyro
git clone https://github.com/krish9219/pyro.git
cd pyro
chmod +x install.sh
./install.sh
```

### Alpine Linux

```bash
# Install prerequisites
apk add g++ cmake git make

# Clone and install Pyro
git clone https://github.com/krish9219/pyro.git
cd pyro
chmod +x install.sh
./install.sh
```

### Verify on Linux

```bash
pyro version
```

Expected output:

```
Pyro Programming Language v1.0.0
Created by Aravind Pilla
Transpiles to C++20 | 76 built-in modules | .ro files
```

---

## macOS

### Apple Silicon (M1/M2/M3/M4) and Intel

```bash
# Step 1: Install Xcode Command Line Tools (provides Apple Clang with C++20)
xcode-select --install

# Step 2: Install CMake via Homebrew
brew install cmake

# Step 3: Clone and install Pyro
git clone https://github.com/krish9219/pyro.git
cd pyro
chmod +x install.sh
./install.sh
```

### Using Homebrew Only

If you prefer not to install Xcode tools:

```bash
brew install gcc cmake git
git clone https://github.com/krish9219/pyro.git
cd pyro
./install.sh
```

### macOS with MacPorts

```bash
sudo port install gcc12 cmake git
git clone https://github.com/krish9219/pyro.git
cd pyro
./install.sh
```

### Verify on macOS

```bash
pyro version
```

---

## Windows

### Recommended: One-Line Install (PowerShell)

```powershell
irm https://aravindlabs.tech/pyro-lang/install.ps1 | iex
```

This installs everything automatically:
- **Pyro compiler** — pre-built, statically linked binary (no DLL dependencies)
- **MinGW g++ toolchain** (~79 MB) — so `pyro run` and `pyro build` work immediately
- Adds both to your user PATH

No Visual Studio, no restarts, no admin needed. Open a **new** PowerShell window after install.

### Verify on Windows

```powershell
pyro --version
# Pyro Programming Language v1.0.0
```

### Build from Source (advanced)

Only needed if you want to modify Pyro itself. Requires Git, CMake, and a C++ compiler.

```powershell
git clone https://github.com/krish9219/pyro.git
cd pyro
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

---

## Building from Source

For all platforms, the manual build process is the same:

```bash
# Clone the repository
git clone https://github.com/krish9219/pyro.git
cd pyro

# Create a build directory
mkdir build && cd build

# Configure with CMake (C++20 is required)
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20

# Build (use -j for parallel compilation)
# Linux/macOS:
make -j$(nproc)
# Windows (MSVC):
cmake --build . --config Release
# Windows (MinGW):
mingw32-make -j%NUMBER_OF_PROCESSORS%
```

The resulting binary will be at:
- Linux/macOS: `build/pyro`
- Windows: `build/Release/pyro.exe` or `build/pyro.exe`

### Install After Building

**Linux / macOS:**

```bash
sudo cp build/pyro /usr/local/bin/
sudo chmod +x /usr/local/bin/pyro
```

**Windows:**
Copy `pyro.exe` to a directory in your PATH (see [PATH Setup](#path-setup)).

### Build Options

```bash
# Debug build (with symbols, slower)
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=20

# Release build (optimized, recommended)
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20

# Specify a custom install prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/pyro

# Build and install in one step
cmake --build . --target install
```

---

## Package Managers

### Homebrew (macOS / Linux)

```bash
brew tap krish9219/pyro
brew install pyro
```

### AUR (Arch Linux)

```bash
yay -S pyro-lang
# or
paru -S pyro-lang
```

### Snap (Ubuntu / Linux)

```bash
sudo snap install pyro-lang
```

### Scoop (Windows)

```powershell
scoop bucket add pyro https://github.com/aravindpilla/scoop-pyro
scoop install pyro
```

### Chocolatey (Windows)

```cmd
choco install pyro-lang
```

### Nix (NixOS / any Linux / macOS)

```bash
nix-env -iA nixpkgs.pyro-lang
```

---

## IDE Extensions and Editor Support

Pyro has syntax highlighting, code completion, and error checking extensions for popular editors. All extensions recognize `.ro` files.

### Visual Studio Code

1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X / Cmd+Shift+X)
3. Search for **"Pyro Language"**
4. Click Install

Or install from the command line:

```bash
code --install-extension aravindpilla.pyro-lang
```

**Features:**
- Syntax highlighting for `.ro` files
- Code completion
- Error diagnostics
- Format on save
- Run file with Ctrl+Shift+B
- Snippet support
- Pipe operator `|>` highlighting

### Vim / Neovim

Add the Pyro plugin to your plugin manager:

**vim-plug:**

```vim
Plug 'krish9219/pyro.vim'
```

**lazy.nvim:**

```lua
{ 'krish9219/pyro.vim' }
```

Then run `:PlugInstall` (vim-plug) or restart Neovim (lazy.nvim).

### Sublime Text

1. Open Command Palette (Ctrl+Shift+P / Cmd+Shift+P)
2. Select "Package Control: Install Package"
3. Search for **"Pyro Syntax"**
4. Install

### JetBrains IDEs (IntelliJ, CLion, etc.)

1. Go to Settings > Plugins
2. Search for **"Pyro Language Support"**
3. Install and restart

### Emacs

Add to your `.emacs` or `init.el`:

```elisp
(use-package pyro-mode
  :ensure t
  :mode "\\.ro\\'")
```

### Helix

Pyro support is built into Helix. Add to `languages.toml`:

```toml
[[language]]
name = "pyro"
scope = "source.ro"
file-types = ["ro"]
comment-token = "#"
indent = { tab-width = 4, unit = "    " }
```

---

## PATH Setup

After building or installing Pyro, make sure the `pyro` binary is in your system PATH.

### Linux / macOS

Add to your `~/.bashrc`, `~/.zshrc`, or `~/.profile`:

```bash
# If installed to /usr/local/bin (default)
# It should already be in PATH. Verify:
which pyro

# If installed elsewhere, add it:
export PATH="/path/to/pyro:$PATH"
```

Apply changes:

```bash
source ~/.bashrc
# or
source ~/.zshrc
```

### Windows

#### GUI Method

1. Press Win+R, type `sysdm.cpl`, press Enter
2. Go to **Advanced** tab > **Environment Variables**
3. Under **System variables**, find **Path** and click **Edit**
4. Click **New** and add the directory containing `pyro.exe`
5. Click OK on all dialogs
6. Restart your terminal

#### Command Line (Admin)

```cmd
setx PATH "%PATH%;C:\path\to\pyro" /M
```

#### PowerShell (Admin)

```powershell
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\path\to\pyro", "Machine")
```

---

## Docker

Run Pyro in a container without installing anything locally:

```bash
# Pull the official Pyro image
docker pull krish9219/pyro:latest

# Run a Pyro program
docker run --rm -v $(pwd):/app krish9219/pyro:latest pyro run /app/hello.ro

# Interactive shell with Pyro
docker run --rm -it -v $(pwd):/app krish9219/pyro:latest bash
```

### Dockerfile for Pyro Projects

```dockerfile
FROM krish9219/pyro:latest

WORKDIR /app
COPY . .

RUN pyro build main.ro -o app

CMD ["./app"]
```

Build and run:

```bash
docker build -t my-pyro-app .
docker run --rm my-pyro-app
```

---

## Verifying Your Installation

After installation, run these commands to confirm everything works:

### Check Version

```bash
pyro version
```

Expected output:

```
Pyro Programming Language v1.0.0
Created by Aravind Pilla
Transpiles to C++20 | 76 built-in modules | .ro files
```

### Run Hello World

Create a file called `hello.ro`:

```pyro
print("Hello, World!")
print("Pyro is installed and working!")
```

Run it:

```bash
pyro run hello.ro
```

Expected output:

```
Hello, World!
Pyro is installed and working!
```

### Build an Executable

```bash
pyro build hello.ro -o hello
./hello     # Linux/macOS
hello.exe   # Windows
```

### View Generated C++

```bash
pyro emit hello.ro
```

This shows the C++20 code that Pyro generates, which is useful for understanding how Pyro works under the hood. You will see `auto` parameters throughout, which is why C++20 is required.

### Test the Built-in Libraries

Create a file called `test_stdlib.ro` to verify the 76 built-in modules are available:

```pyro
import math
import json
import time

print("math.PI = " + str(math.PI))
print("Current time: " + time.format(time.now(), "YYYY-MM-DD HH:mm:ss"))

let obj = {"language": "Pyro", "modules": 76}
print(json.stringify(obj))
print("All 76 built-in modules ready!")
```

```bash
pyro run test_stdlib.ro
```

---

## Pyro Commands

| Command | Description |
|---------|-------------|
| `pyro run <file>` | Compile and run a `.ro` program |
| `pyro build <file>` | Compile to a standalone executable |
| `pyro build <file> -o <name>` | Compile with a custom output name |
| `pyro emit <file>` | Display the generated C++20 code |
| `pyro tokens <file>` | Show lexer tokens (for debugging) |
| `pyro ast <file>` | Show the abstract syntax tree (for debugging) |
| `pyro version` | Show version information |
| `pyro help` | Show usage help |

### Examples

```bash
# Run a program
pyro run examples/hello.ro

# Build an optimized executable
pyro build main.ro -o myapp

# See what C++20 gets generated
pyro emit examples/web_server.ro

# Debug: see how Pyro tokenizes your code
pyro tokens examples/basics.ro
```

---

## Updating Pyro

### From Git

```bash
cd pyro
git pull origin main
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
make -j$(nproc)
sudo make install
```

### Homebrew

```bash
brew upgrade pyro
```

### Snap

```bash
sudo snap refresh pyro-lang
```

### Scoop (Windows)

```powershell
scoop update pyro
```

---

## Uninstalling

### If Installed with install.sh

```bash
sudo rm /usr/local/bin/pyro
sudo rm -rf /usr/local/lib/pyro
```

### Homebrew

```bash
brew uninstall pyro
```

### Snap

```bash
sudo snap remove pyro-lang
```

### Scoop (Windows)

```powershell
scoop uninstall pyro
```

### Manual (Windows)

Delete `pyro.exe` from wherever you placed it, and remove that directory from your PATH environment variable.

---

## Troubleshooting

### "Command not found: pyro"

Pyro is not in your PATH. See [PATH Setup](#path-setup).

Verify the binary exists:

```bash
# Linux/macOS
ls -la /usr/local/bin/pyro

# Or find it
find / -name "pyro" -type f 2>/dev/null
```

### "No C++20 compiler found"

Pyro requires C++20 because the transpiled output uses `auto` parameters for dynamic typing. Install a modern C++ compiler:

| Platform | Command |
|----------|---------|
| Ubuntu/Debian | `sudo apt install g++-10` (or newer) |
| Fedora | `sudo dnf install gcc-c++` |
| Arch | `sudo pacman -S gcc` |
| macOS | `xcode-select --install` |
| Windows | Install Visual Studio 2019+ Build Tools |

Verify C++20 support:

```bash
g++ -std=c++20 -o test test.cpp
# or
clang++ -std=c++20 -o test test.cpp
```

### "CMake not found" or CMake version too old

| Platform | Command |
|----------|---------|
| Ubuntu/Debian | `sudo apt install cmake` |
| Fedora | `sudo dnf install cmake` |
| Arch | `sudo pacman -S cmake` |
| macOS | `brew install cmake` |
| Windows | Download from [cmake.org](https://cmake.org/download/) |

If your system cmake is too old:

```bash
pip install cmake    # installs latest cmake via pip
```

### "Permission denied" during install

Use `sudo` for system-wide installation:

```bash
sudo ./install.sh
```

Or install to a user-local directory:

```bash
mkdir -p ~/.local/bin
cp build/pyro ~/.local/bin/
# Add ~/.local/bin to PATH in your shell config
```

### Build errors in generated C++ code

1. Check your compiler version: `g++ --version` or `clang++ --version`
2. Ensure C++20 support: try `g++ -std=c++20 -o test test.cpp`
3. View the generated C++ for debugging: `pyro emit <file>`
4. Report issues at the Pyro repository with the output of `pyro emit`

### macOS: "xcrun: error: invalid active developer path"

```bash
xcode-select --install
```

### Windows: "cl is not recognized"

Run commands from **Developer Command Prompt for Visual Studio**, not regular Command Prompt. Or add MSVC to your PATH.

### Slow compilation

Pyro compilation should be fast. If it is slow:

```bash
# Use parallel make
make -j$(nproc)          # Linux
make -j$(sysctl -n hw.ncpu)  # macOS

# Or with CMake
cmake --build . --parallel
```

---

*Pyro Installation Guide — Created by **Aravind Pilla***
*"Getting started should take minutes, not hours."*
