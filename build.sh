#!/bin/bash
# Pyro Programming Language - Build Script
# Created by Aravind Pilla

set -e

BOLD='\033[1m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}${BOLD}"
echo "  ____                    "
echo " |  _ \ _   _ _ __ ___   "
echo " | |_) | | | | '__/ _ \  "
echo " |  __/| |_| | | | (_) | "
echo " |_|    \__, |_|  \___/  "
echo "        |___/             "
echo ""
echo -e " Pyro Programming Language${NC}"
echo -e " Created by Aravind Pilla"
echo -e " The language that makes Python jealous."
echo ""

# Check prerequisites
echo -e "${BOLD}Checking prerequisites...${NC}"

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: No C++20 compiler found. Install g++ or clang++.${NC}"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake not found. Install cmake 3.14+.${NC}"
    exit 1
fi

echo -e "${GREEN}Prerequisites OK${NC}"
echo ""

# Build
echo -e "${BOLD}Building Pyro compiler...${NC}"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
cd ..

echo ""
echo -e "${GREEN}${BOLD}Build successful!${NC}"
echo ""
echo "Binary: build/pyro"
echo ""
echo "To install system-wide:"
echo "  sudo cp build/pyro /usr/local/bin/"
echo ""
echo "Quick test:"
echo "  ./build/pyro run examples/hello.ro"
echo ""
