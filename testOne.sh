#!/bin/bash
# testOne.sh functional test script $Revision$
# Call with test platform, test-program, arguments
# e.g. "./testOne.sh pentium hello"
# or   "./testOne.sh sparc fibo 10" 
echo $* > functest.res
rm -f functest/$2.c
./boomerang -o functest test/$1/$2 2>/dev/null >/dev/null
ret=$?
if [[ ret -ge 128 ]]; then
    echo Result for $1 $2: Boomerang failed with signal $((ret-128))
      >> functest.res
else
    if [[ ! -f functest/$2/$2.c ]]; then
        echo Result for $1 $2: No boomerang output! >> functest.res
    else
        cp functest/$2/$2.c functest.c
        # if test/$1/$2.sed exists, use it to make "known error" corrections
        # to the source code
        if [[ -f test/$1/$2.sed ]]; then
            echo Warning... $1/$2.sed used >> functest.res
            sed -f test/$1/$2.sed functest.c > functest.tmp
            ret=$?
            if [[ ret -ne 0 ]]; then
                echo test/$1/$2.sed failed! >> functest.res
                echo
                exit 10
            fi
            mv functest.tmp functest.c
        fi
        gcc -o functest.exe functest.c >> functest.res 2>&1
        if [[ $? != 0 ]]; then
            echo Result for $1 $2: Compile failed >> functest.res
        else
            rm -f functest.out
            ./functest.exe $3 $4 $5 $6 $7 $8 $9 >> functest.out 2>&1
            ret=$?
            if [[ ret -ge 128 ]]; then
                echo Result for $1 $2: Execution terminated with signal \
                  $((ret-128)) >> functest.res
            else
                if [[ ret -ne 0 ]]; then
                    echo Warning! return code from execute was $((ret)) >> \
                      functest.res
                fi
                diff -c test/source/$2.out functest.out > functest.tmp
                ret=$?
                # Filter out control chars that may happen due to bad decomp.
                tr -d < functest.tmp [:cntrl:] >> functest.res
                if [[ ret -ne 0 ]]; then
                    echo Result for $1 $2: failed diff >> functest.res
                else
                    echo Result for $1 $2: Passed >> functest.res
                fi
            fi
        fi
    fi
fi
echo >> functest.res
cat functest.res >> functests.out
grep "^Result" functest.res
echo
