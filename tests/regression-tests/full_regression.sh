#!/bin/bash
#./test_use_all.sh

if hash ruby 2>/dev/null; then
  ./regression_tester.rb ./out/boomerang
elif hash python 2>/dev/null; then
  ./regression_tester.py ./out/boomerang
else
  echo "$0 requires ruby or python to run"
  exit 1
fi

#diff -rwB tests/baseline/ tests/outputs/


