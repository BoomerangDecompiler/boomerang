#!/bin/bash
# functest.sh functional test script $Revision$
# Note: to test with data flow based type analysis, define environment variable BOOMSW to be -Td
if [ -z $BOOMSW ]; then BOOMSW="--"; fi
echo Boomerang switch BOOMSW is $BOOMSW
rm -rf functest
mkdir functest
rm -rf functests.out
./testOne.sh pentium hello $BOOMSW
./testOne.sh sparc   hello $BOOMSW
./testOne.sh pentium twoproc $BOOMSW
./testOne.sh sparc   twoproc $BOOMSW
./testOne.sh pentium fib $BOOMSW
./testOne.sh sparc   fib $BOOMSW
./testOne.sh pentium fibo-O4 $BOOMSW < test/source/fibo-O4.in
./testOne.sh sparc   fibo-O4 $BOOMSW < test/source/fibo-O4.in
./testOne.sh pentium global1 $BOOMSW
./testOne.sh sparc   global1 $BOOMSW
./testOne.sh pentium global2 $BOOMSW
./testOne.sh sparc   global2 $BOOMSW
./testOne.sh pentium global3 $BOOMSW
./testOne.sh sparc   global3 $BOOMSW
./testOne.sh pentium switch_gcc $BOOMSW 2 3 4 5
./testOne.sh sparc   switch_gcc $BOOMSW 2 3 4 5
./testOne.sh pentium switch_cc $BOOMSW 2 3 4
./testOne.sh sparc   switch_cc $BOOMSW 2 3 4
./testOne.sh pentium stattest $BOOMSW
./testOne.sh sparc   stattest $BOOMSW
./testOne.sh pentium minmax $BOOMSW 2 3 4
./testOne.sh sparc   minmax $BOOMSW 2 3 4
./testOne.sh pentium minmax2 $BOOMSW two
./testOne.sh sparc   minmax2 $BOOMSW two
./testOne.sh pentium minmax3 $BOOMSW two
./testOne.sh pentium printpi $BOOMSW
./testOne.sh sparc   printpi $BOOMSW
./testOne.sh pentium uns $BOOMSW 2 3
./testOne.sh sparc   uns $BOOMSW 2 3
./testOne.sh pentium fromssa2 $BOOMSW
./testOne.sh sparc   fromssa2 $BOOMSW
./testOne.sh pentium sumarray-O4 $BOOMSW
./testOne.sh sparc   sumarray-O4 $BOOMSW
./testOne.sh pentium bswap $BOOMSW
./testOne.sh pentium testset -O 2 3 4
echo === Done ===
