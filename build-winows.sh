#!/bin/bash

set -e

mkdir -p .out
mkdir -p .build-windows && cd .build-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake
cmake --build .
cmake --install . --prefix ../.out-windows/
cd ..