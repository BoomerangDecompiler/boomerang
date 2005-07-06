#!/bin/bash
# functest.sh functional test script $Revision$	# 1.23.2.1
# Note: to test with data flow based type analysis, pass a parameter of -Td
#
# 02 Feb 05 - Mike: Conditional tests for no type analysis. So all tests should pass whether -Td is passed or not
# 06 Feb 05 - Mike: Pass the test-set parameter to testOne.sh
#
if [ -z $1 ]; then BOOMSW="--"; else BOOMSW=$1; fi
echo Boomerang switch is $BOOMSW

rm -rf functest
mkdir functest
rm -rf functests.out

./testOne.sh pentium hello 1 $BOOMSW
./testOne.sh sparc   hello 1 $BOOMSW
./testOne.sh ppc	 hello 1 $BOOMSW
./testOne.sh pentium twoproc 1 $BOOMSW
./testOne.sh sparc   twoproc 1 $BOOMSW
./testOne.sh ppc	 twoproc 1 $BOOMSW
./testOne.sh pentium fib 1 $BOOMSW
./testOne.sh sparc   fib 1 $BOOMSW
./testOne.sh pentium fibo-O4 1 $BOOMSW < test/source/fibo-O4.in1
./testOne.sh sparc   fibo-O4 1 $BOOMSW < test/source/fibo-O4.in1
./testOne.sh pentium fibo-O4 3 $BOOMSW < test/source/fibo-O4.in3
./testOne.sh sparc   fibo-O4 3 $BOOMSW < test/source/fibo-O4.in3
./testOne.sh pentium global1 1 $BOOMSW
./testOne.sh sparc   global1 1 $BOOMSW
./testOne.sh pentium global2 1 $BOOMSW
./testOne.sh sparc   global2 1 $BOOMSW
./testOne.sh pentium global3 1 $BOOMSW
./testOne.sh sparc   global3 1 $BOOMSW
if [ $BOOMSW == "--" ]
then
  echo Skipping switch tests, requires type analysis
  echo
else
./testOne.sh pentium switch_gcc 5 $BOOMSW 2 3 4 5
./testOne.sh sparc   switch_gcc 5 $BOOMSW 2 3 4 5
./testOne.sh pentium switch_cc 4 $BOOMSW 2 3 4
./testOne.sh sparc   switch_cc 4 $BOOMSW 2 3 4
./testOne.sh pentium switch_gcc 1 $BOOMSW
./testOne.sh sparc   switch_gcc 1 $BOOMSW
./testOne.sh pentium switch_cc 1 $BOOMSW
./testOne.sh sparc   switch_cc 1 $BOOMSW
fi
if [ $BOOMSW == "--" ]
then
  echo Skipping stattest, now requires type analysis
  echo
else
./testOne.sh pentium stattest 1 $BOOMSW
./testOne.sh sparc   stattest 1 $BOOMSW
fi
./testOne.sh pentium minmax 1 $BOOMSW 2 3 4
./testOne.sh sparc   minmax 1 $BOOMSW 2 3 4
./testOne.sh pentium minmax2 1 $BOOMSW two
./testOne.sh sparc   minmax2 1 $BOOMSW two
./testOne.sh pentium minmax3 1 $BOOMSW two
if [ $BOOMSW == "--" ]
then
  echo Skipping printpi tests, requires type analysis
  echo
else
./testOne.sh pentium printpi 1 $BOOMSW
./testOne.sh sparc   printpi 1 $BOOMSW
fi
./testOne.sh pentium uns 1 $BOOMSW 2 3
./testOne.sh sparc   uns 1 $BOOMSW 2 3
./testOne.sh pentium fromssa2 1 $BOOMSW
./testOne.sh sparc   fromssa2 1 $BOOMSW
./testOne.sh pentium sumarray-O4 1 $BOOMSW
if [ $BOOMSW == "--" ]
then
  echo Skipping sparc sumarray test, requires type analysis
  echo
else
./testOne.sh sparc   sumarray-O4 1 $BOOMSW
fi
./testOne.sh pentium bswap 1 $BOOMSW
./testOne.sh pentium testset 1 -O 2 3 4
./testOne.sh sparc   andn 1 $BOOMSW

#./testOne.sh pentium line1 1 $BOOMSW test/source/line1.c
#./testOne.sh sparc   line1 1 $BOOMSW test/source/line1.c

echo === Done ===
