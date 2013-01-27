#!/bin/bash
#./test_use_all.sh
./regression_tester.rb ./out/boomerang; diff -wB tests/baseline/ tests/outputs/
