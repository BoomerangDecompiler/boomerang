#!/bin/bash
set -e

cd build
cmake ..
make -j$(nproc)
