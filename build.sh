#!/usr/bin/env bash
set -e

BUILD_DIR="build"

echo "==> Cleaning..."
rm -rf "$BUILD_DIR"

echo "==> Configuring with CMake..."

cmake -B "$BUILD_DIR" -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native -mtune=native -flto -funroll-loops -pipe" \
  -DCMAKE_C_FLAGS_RELEASE="-O3 -march=native -mtune=native -flto -funroll-loops -pipe" \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON

echo "==> Building..."
cmake --build "$BUILD_DIR" -j$(nproc)

echo "==> Done: $BUILD_DIR"