#!/bin/bash

set -e

mkdir -p .out
mkdir -p .build && cd .build
cmake ..
cmake --build .
cmake --install . --prefix ../.out/
cd ..
./.out/bin/examples