#!/bin/bash

set -e
echo "building..."
cmake -B build
cd build
echo "compiling..."
make
echo "running..."
echo
./chess_engine
