#!/bin/bash

set -e
echo "building..."
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cd build
echo "compiling..."
make
echo "running..."
echo
./chess_engine
