#!/bin/bash
# Pyro Programming Language - Installer
# Created by Aravind Pilla
# Supports: Linux, macOS, WSL

set -e

BOLD='\033[1m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

INSTALL_DIR="/usr/local"
PYRO_LIB_DIR="/usr/local/lib/pyro"

echo -e "${BLUE}${BOLD}"
echo "  ____                    "
echo " |  _ \ _   _ _ __ ___   "
echo " | |_) | | | | '__/ _ \  "
echo " |  __/| |_| | | | (_) | "
echo " |_|    \__, |_|  \___/  "
echo "        |___/             "
echo ""
echo -e " Pyro Programming Language Installer${NC}"
echo -e " Created by Aravind Pilla"
echo ""

# Detect OS
OS="$(uname -s)"
case "${OS}" in
    Linux*)     PLATFORM=Linux;;
    Darwin*)    PLATFORM=macOS;;
    CYGWIN*|MINGW*|MSYS*) PLATFORM=Windows;;
    *)          PLATFORM="Unknown";;
esac
echo -e "Platform: ${BOLD}${PLATFORM}${NC}"

# Check prerequisites
echo -e "\n${BOLD}Checking prerequisites...${NC}"

# Check C++ compiler
CXX=""
if command -v g++ &> /dev/null; then
    CXX="g++"
    CXX_VERSION=$(g++ --version | head -n1)
    echo -e "  C++ compiler: ${GREEN}${CXX_VERSION}${NC}"
elif command -v clang++ &> /dev/null; then
    CXX="clang++"
    CXX_VERSION=$(clang++ --version | head -n1)
    echo -e "  C++ compiler: ${GREEN}${CXX_VERSION}${NC}"
else
    echo -e "  ${RED}No C++17 compiler found!${NC}"
    echo ""
    if [ "$PLATFORM" = "Linux" ]; then
        echo "  Install with: sudo apt install g++ build-essential"
    elif [ "$PLATFORM" = "macOS" ]; then
        echo "  Install with: xcode-select --install"
    fi
    exit 1
fi

# Check CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1)
    echo -e "  CMake: ${GREEN}${CMAKE_VERSION}${NC}"
else
    echo -e "  ${RED}CMake not found!${NC}"
    echo ""
    if [ "$PLATFORM" = "Linux" ]; then
        echo "  Install with: sudo apt install cmake"
    elif [ "$PLATFORM" = "macOS" ]; then
        echo "  Install with: brew install cmake"
    fi
    exit 1
fi

echo -e "\n${BOLD}Building Pyro...${NC}"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

echo -e "\n${BOLD}Installing Pyro...${NC}"
if [ -w "${INSTALL_DIR}/bin" ]; then
    make install
else
    echo -e "${YELLOW}Requires sudo for installation to ${INSTALL_DIR}${NC}"
    sudo make install
fi

cd ..

# Create stdlib directory
sudo mkdir -p "${PYRO_LIB_DIR}/stdlib"
sudo cp -r stdlib/* "${PYRO_LIB_DIR}/stdlib/" 2>/dev/null || true

echo -e "\n${GREEN}${BOLD}Pyro installed successfully!${NC}"
echo ""
echo -e "  Binary:   ${INSTALL_DIR}/bin/nova"
echo -e "  Stdlib:   ${PYRO_LIB_DIR}/stdlib/"
echo ""
echo -e "  Test with: ${BOLD}pyro version${NC}"
echo -e "  Run:       ${BOLD}pyro run examples/hello.ro${NC}"
echo ""
echo -e "  ${BLUE}Happy coding with Pyro! - Aravind Pilla${NC}"
echo ""
