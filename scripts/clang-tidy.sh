#!/bin/bash
set -e

for f in $(find src -type f -name "*.cpp" -o -name "*.h" | grep -iv scanner | grep -iv parser | grep -iv macho | sort -u)
do
    echo $f
    clang-tidy -p build/compile_commands.json $f
done
