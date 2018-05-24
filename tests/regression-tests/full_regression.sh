#!/bin/bash
#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# exepath=$1
# workingDir=$2
if hash python3 2>/dev/null; then
    python3 ./regression_tester.py $@
else
    echo "$0 requires Python 3 to run"
    exit 1
fi
