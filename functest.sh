#!/bin/bash
rm -rf functest
mkdir functest
rm -rf functests.out
./testOne.sh pentium hello
./testOne.sh sparc hello
./testOne.sh pentium  twoproc
./testOne.sh sparc twoproc
./testOne.sh pentium  fib
./testOne.sh pentium fibo-O4 < test/source/fibo-O4.in
./testOne.sh sparc fibo-O4 < test/source/fibo-O4.in
./testOne.sh pentium global1
./testOne.sh sparc global1
./testOne.sh pentium  global2
./testOne.sh sparc global2
./testOne.sh pentium  global3
./testOne.sh sparc global3
./testOne.sh pentium  switch_gcc 2 3 4 5
./testOne.sh sparc switch_gcc 2 3 4 5
./testOne.sh pentium  switch_cc 2 3 4
./testOne.sh sparc switch_cc 2 3 4
./testOne.sh pentium stattest
./testOne.sh parc stattest
./testOne.sh pentium minmax 2 3 4
./testOne.sh sparc minmax 2 3 4
./testOne.sh pentium minmax2 two
./testOne.sh sparc minmax2 two
./testOne.sh pentium minmax3 two
echo === Done ===
