#!/bin/bash

UNC_FILES="$(find ../src/boomerang/codegen  -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang/core      -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang/db        -type f -name '*.cpp' -o -name '*.h' | grep -iv '/ssl/ssl') \
    $(find ../src/boomerang/frontend  -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang/loader    -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang/passes    -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang/transform -type f -name '*.cpp' -o -name '*.h' | grep -v 'transformation') \
    $(find ../src/boomerang/type      -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang/util      -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang-cli       -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang-gui       -type f -name '*.cpp' -o -name '*.h') \
    $(find ../src/boomerang-loaders   -type f -name '*.c' -o -name '*.cpp' -o -name '*.h') \
    $(find ../tests/unit-tests        -type f -name '*.c' -o -name '*.cpp' -o -name '*.h')"

for file in $UNC_FILES; do
    echo "Uncrustifying $file ..."
    uncrustify -c ../uncrustify.cfg --no-backup "$file" &>/dev/null &
done

wait
