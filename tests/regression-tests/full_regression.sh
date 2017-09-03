#!/bin/bash
#./test_use_all.sh

# exepath=$1
# workingDir=$2

if hash python3 2>/dev/null; then
    python3 ./regression_tester.py $@
else
    echo "$0 requires Python 3 to run"
    exit 1
fi

#diff -rwB tests/baseline/ tests/outputs/
