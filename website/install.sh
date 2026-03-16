#!/bin/bash
# Pyro Programming Language — One-Line Installer
# P.Y.R.O — Performance You Really Own
# Created by Aravind Pilla

set -e

REPO="krish9219/pyro"
INSTALL_DIR="$HOME/.pyro/bin"

echo ""
echo "  🔥 Pyro Installer"
echo "  P.Y.R.O — Performance You Really Own"
echo ""

# Detect OS
OS="$(uname -s)"
ARCH="$(uname -m)"

if [ "$OS" = "Linux" ] && [ "$ARCH" = "x86_64" ]; then
    PLATFORM="linux-x86_64"
elif [ "$OS" = "Darwin" ] && [ "$ARCH" = "arm64" ]; then
    PLATFORM="macos-arm64"
elif [ "$OS" = "Darwin" ] && [ "$ARCH" = "x86_64" ]; then
    PLATFORM="macos-x86_64"
else
    echo "  Building from source for $OS $ARCH..."
    echo ""

    # Check prerequisites
    if ! command -v g++ &>/dev/null && ! command -v clang++ &>/dev/null; then
        echo "  Error: No C++ compiler found. Install g++ or clang++."
        exit 1
    fi
    if ! command -v cmake &>/dev/null; then
        echo "  Error: CMake not found. Install cmake."
        exit 1
    fi

    cd /tmp
    rm -rf pyro-build
    git clone --depth 1 https://github.com/$REPO.git pyro-build
    cd pyro-build
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

    mkdir -p "$INSTALL_DIR"
    cp pyro "$INSTALL_DIR/"
    cd / && rm -rf /tmp/pyro-build

    echo "  ✓ Built and installed to $INSTALL_DIR/pyro"
    PLATFORM="source"
fi

if [ "$PLATFORM" != "source" ]; then
    echo "  Downloading Pyro for $PLATFORM..."
    mkdir -p "$INSTALL_DIR"

    # Try GitHub release first, fall back to building from source
    RELEASE_URL="https://github.com/$REPO/releases/latest/download/pyro-$PLATFORM"
    if curl -fsSL "$RELEASE_URL" -o "$INSTALL_DIR/pyro" 2>/dev/null; then
        chmod +x "$INSTALL_DIR/pyro"
        echo "  ✓ Downloaded to $INSTALL_DIR/pyro"
    else
        echo "  Pre-built binary not available. Building from source..."
        cd /tmp
        rm -rf pyro-build
        git clone --depth 1 https://github.com/$REPO.git pyro-build
        cd pyro-build && mkdir build && cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
        cp pyro "$INSTALL_DIR/"
        cd / && rm -rf /tmp/pyro-build
        echo "  ✓ Built and installed to $INSTALL_DIR/pyro"
    fi
fi

# Add to PATH
SHELL_RC=""
if [ -f "$HOME/.zshrc" ]; then SHELL_RC="$HOME/.zshrc"
elif [ -f "$HOME/.bashrc" ]; then SHELL_RC="$HOME/.bashrc"
elif [ -f "$HOME/.profile" ]; then SHELL_RC="$HOME/.profile"
fi

if [ -n "$SHELL_RC" ]; then
    if ! grep -q '.pyro/bin' "$SHELL_RC" 2>/dev/null; then
        echo 'export PATH="$HOME/.pyro/bin:$PATH"' >> "$SHELL_RC"
        echo "  ✓ Added to PATH in $SHELL_RC"
    fi
fi

export PATH="$HOME/.pyro/bin:$PATH"

echo ""
echo "  ✓ Pyro installed successfully!"
echo ""
echo "  Run: pyro version"
echo "  Try: echo 'print(\"Hello, Pyro!\")' > hello.ro && pyro run hello.ro"
echo ""
echo "  Restart your terminal or run: source $SHELL_RC"
echo ""
