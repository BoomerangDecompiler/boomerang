#!/usr/bin/env python3
#
# This file is part of the Boomerang Decompiler.
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

import os
import shutil
import subprocess
import sys
import difflib

from collections import defaultdict
from filecmp import dircmp


# These files are used for checking for regressions
regression_tests = [
    "elf32-ppc/fibo",
    "elf32-ppc/hello",
    "elf32-ppc/minmax",
    "elf32-ppc/switch",

    "elf/hello-clang4-dynamic",

    "OSX/banner",
    "OSX/branch",
    "OSX/condcodexform",
    "OSX/daysofxmas",
    "OSX/fbranch",
    "OSX/fib",
    "OSX/fibo2",
    "OSX/fibo_iter",
    "OSX/fromssa2",
    "OSX/frontier",
    "OSX/funcptr",
    "OSX/global1",
    "OSX/global2",
    "OSX/global3",
    "OSX/hello",
    "OSX/ifthen",
    "OSX/loop",
    "OSX/manyparams",
    "OSX/minmax",
    "OSX/minmax2",
    "OSX/ohello",
    "OSX/paramchain",
    "OSX/phi",
    "OSX/phi2",
    "OSX/printpi",
    "OSX/semi",
    "OSX/set",
    "OSX/stattest",
    "OSX/sumarray",
    "OSX/superstat",
    "OSX/switch",
    "OSX/twoproc",
    "OSX/twoproc2",
    "OSX/uns",

    "OSX/o4/banner",
    "OSX/o4/branch",
    "OSX/o4/condcodexform",
    "OSX/o4/daysofxmas",
    "OSX/o4/fib",
    "OSX/o4/fibo",
    "OSX/o4/fibo2",
    "OSX/o4/fibo_iter",
    "OSX/o4/fromssa2",
    "OSX/o4/frontier",
    "OSX/o4/funcptr",
    "OSX/o4/global1",
    "OSX/o4/global2",
    "OSX/o4/global3",
    "OSX/o4/hello",
    "OSX/o4/ifthen",
    "OSX/o4/loop",
    "OSX/o4/manyparams",
    "OSX/o4/minmax",
    "OSX/o4/minmax2",
    "OSX/o4/paramchain",
    "OSX/o4/phi",
    "OSX/o4/phi2",
    "OSX/o4/printpi",
    "OSX/o4/semi",
    "OSX/o4/set",
    "OSX/o4/stattest",
    "OSX/o4/sumarray",
    "OSX/o4/superstat",
    "OSX/o4/switch",
    "OSX/o4/twoproc",
    "OSX/o4/twoproc2",
    "OSX/o4/uns",

    "ppc/banner",
    "ppc/branch",
    "ppc/condcodexform",
    "ppc/daysofxmas",
    "ppc/fbranch",
    "ppc/fib",
    "ppc/fibo",
    "ppc/fibo2",
    "ppc/fibo_iter",
    "ppc/fromssa2",
    "ppc/frontier",
    "ppc/funcptr",
    "ppc/global1",
    "ppc/global2",
    "ppc/global3",
    "ppc/hello",
    "ppc/ifthen",
    "ppc/loop",
    "ppc/manyparams",
    "ppc/minmax",
    "ppc/minmax2",
    "ppc/paramchain",
    "ppc/phi",
    "ppc/phi2",
    "ppc/printpi",
    "ppc/semi",
    "ppc/set",
    "ppc/stattest",
    "ppc/sumarray",
    "ppc/superstat",
    "ppc/switch",
    "ppc/twoproc",
    "ppc/twoproc2",
    "ppc/uns",

    "ppc/o4/banner",
    "ppc/o4/condcodexform",
    "ppc/o4/daysofxmas",
    "ppc/o4/fib",
    "ppc/o4/fibo",
    "ppc/o4/fibo2",
    "ppc/o4/fibo_iter",
    "ppc/o4/fromssa2",
    "ppc/o4/frontier",
    "ppc/o4/funcptr",
    "ppc/o4/global1",
    "ppc/o4/global2",
    "ppc/o4/global3",
    "ppc/o4/hello",
    "ppc/o4/ifthen",
    "ppc/o4/loop",
    "ppc/o4/manyparams",
    "ppc/o4/minmax",
    "ppc/o4/minmax2",
    "ppc/o4/paramchain",
    "ppc/o4/phi",
    "ppc/o4/phi2",
    "ppc/o4/printpi",
    "ppc/o4/semi",
    "ppc/o4/set",
    "ppc/o4/stattest",
    "ppc/o4/sumarray",
    "ppc/o4/superstat",
    "ppc/o4/switch",
    "ppc/o4/twoproc",
    "ppc/o4/twoproc2",
    "ppc/o4/uns",

    "x86/asgngoto",
    "x86/ass2.Linux",
    "x86/ass3.Linux",
    "x86/banner",
    "x86/branch",
    "x86/branch-linux",
    "x86/bswap",
    "x86/callchain",
    "x86/chararray",
    "x86/chararray-O4",
    "x86/daysofxmas",
    "x86/encrypt",
    "x86/fbranch",
    "x86/fbranch2",
    "x86/fbranch_sahf",
    "x86/fedora2_true",
    "x86/fedora3_true",
    "x86/fib",
    "x86/fibo2",
    "x86/fibo3",
    "x86/fibo4",
    "x86/fibo_iter",
    "x86/fibo-O4",
    "x86/fromssa2",
    "x86/frontier",
    "x86/funcptr",
    "x86/global1",
    "x86/global2",
    "x86/global3",
    "x86/hello",
    "x86/ifthen",
    "x86/line1",
    "x86/line1-o4",
    "x86/localarray",
    "x86/localarray-O4",
    "x86/loop",
    "x86/manyparams",
    "x86/minmax",
    "x86/minmax2",
    "x86/minmax3",
    "x86/nestedswitch",
    "x86/param1",
    "x86/paramchain",
    "x86/phi",
    "x86/phi2",
    "x86/printpi",
    "x86/recursion",
    "x86/recursion2",
    "x86/regalias",
    "x86/regalias2",
    "x86/restoredparam",
    "x86/rux_encrypt",
    "x86/semi",
    "x86/set",
    "x86/shared2",
    "x86/short1",
    "x86/short2",
    "x86/stattest",
    "x86/sumarray",
    "x86/sumarray-O4",
    "x86/superstat",
    "x86/suse_true",
    "x86/switch_cc",
    "x86/switch_gcc",
    "x86/testarray1",
    "x86/testarray2",
    "x86/testset",
    "x86/twofib",
    "x86/twoproc",
    "x86/twoproc2",
    "x86/twoproc3",
    "x86/uns",

    "windows/typetest.exe"
]

# These files are used for checking for crashes or failures only
smoke_tests = [
    # These files cannot be checked for regressions
    # because they have non-deterministic output
    "dos/BENCHFN.EXE",
    "dos/BENCHLNG.EXE",
    "dos/BENCHMUL.EXE",
    "dos/BENCHMUS.EXE",
    "dos/BENCHSHO.EXE",
    "dos/BYTEOPS.EXE",
    "dos/DHAMP.EXE",
    "dos/FIBOL.EXE",
    "dos/FIBOS.EXE",
    "dos/INTOPS.EXE",
    "dos/LONGOPS.EXE",
    "dos/MATRIXMU.EXE",
    "dos/MAX.EXE",
    "dos/STRLEN.EXE",
    "dos/TESTLONG.EXE",

    "OSX/o4/fbranch",

    "ppc/o4/branch",

    "windows/fbranch.exe",
    "windows/hello_release.exe",
    "windows/switch_borland.exe",
    "windows/switch_gcc.exe",
    "windows/switch_msvc5.exe"
]

# These files are disabled explicitly because decompilation fails for them.
disabled_tests = [
    "elf/hello-clang4-static",
    "windows/hello.exe"        # Does not fail, but takes too long
]


""" Clean output directories from old data. """
def clean_old_outputs(base_dir):
    print("Cleaning up old data ...")
    output_dir = os.path.join(base_dir, "outputs")
    if os.path.isdir(output_dir): shutil.rmtree(output_dir, ignore_errors=True)
    os.makedirs(output_dir)



""" Compare directories and print the differences of file content. Returns True if the directories are equal. """
def compare_directories(dir_expected, dir_actual):
    def compare_directories_internal(dcmp):
        directories_equal = True

        for different_file_name in dcmp.diff_files:
            # Found different file
            directories_equal = False

            with open(os.path.join(dcmp.left,  different_file_name), 'r') as file_expected, \
                 open(os.path.join(dcmp.right, different_file_name), 'r') as file_actual:
                diff = difflib.unified_diff(file_expected.readlines(), file_actual.readlines(),
                    fromfile=file_expected.name,
                    tofile  =file_expected.name)

                print("")
                sys.stdout.flush()
                for line in diff:
                    sys.stderr.write(line)
                sys.stderr.flush()
                print("")
                sys.stdout.flush();

        for sub_dcmp in dcmp.subdirs.values():
            directories_equal &= compare_directories_internal(sub_dcmp)

        return directories_equal

    dcmp = dircmp(dir_expected, dir_actual)
    return compare_directories_internal(dcmp)



""" Perform the actual test on a single input binary """
def test_single_input(cli_path, input_file, output_path, expected_output_path, args):
    cmdline   = [cli_path] + ['-P', os.path.dirname(cli_path), '-o', output_path] + args + [input_file]

    try:
        with open(os.path.join(output_path, os.path.basename(input_file) + ".stdout"), "w") as test_stdout, \
             open(os.path.join(output_path, os.path.basename(input_file) + ".stderr"), "w") as test_stderr:

            try:
                result = subprocess.call(cmdline, stdout=test_stdout, stderr=test_stderr, timeout=360)
                result = '.' if result == 0 else 'f'

                if result == '.' and expected_output_path != "":
                    # Perform regression diff
                    if not compare_directories(expected_output_path, output_path):
                        result = 'r'

            except KeyboardInterrupt:
                print("\nAborting regression tests at user request\n")
                sys.exit(2)
            except:
                result = '!'

        return [result, ' '.join(cmdline), input_file]
    except IOError:
        return ['d', ' '.join(cmdline), input_file]



""" Perform regression tests on inputs in test_list. Returns true on success (no regressions). """
def perform_regression_tests(base_dir, test_input_base, test_list):
    test_results = defaultdict();

    sys.stdout.write("Testing for regressions ")
    for test_file in test_list:
        input_file = os.path.join(test_input_base, test_file)
        expected_output_dir = os.path.join(base_dir, "expected-outputs", test_file)
        output_dir = os.path.join(base_dir, "outputs", test_file)
        os.makedirs(output_dir)

        test_result = test_single_input(sys.argv[1], input_file, output_dir, expected_output_dir, sys.argv[2:])
        test_results[test_file] = test_result

        sys.stdout.write(test_result[0]) # print status
        sys.stdout.flush()

    num_failed = sum(1 for res in test_results.values() if res[0] != '.')

    print("")
    if num_failed != 0:
        print("\nRegressions:")
        for res in test_results.values():
            if res[0] != '.':
                sys.stdout.write(res[0] + " " + res[2] + "\n")
                sys.stdout.flush()
        print("")

    sys.stdout.flush()
    return num_failed == 0



""" Perform regression tests on inputs in test_list. Returns true on success (no regressions). """
def perform_smoke_tests(base_dir, test_input_base, test_list):
    test_results = defaultdict();

    sys.stdout.write("Testing for crashes ")
    sys.stdout.flush()

    for test_file in test_list:
        input_file = os.path.join(test_input_base, test_file)
        output_dir = os.path.join(base_dir, "outputs", test_file)
        os.makedirs(output_dir)

        test_result = test_single_input(sys.argv[1], input_file, output_dir, "", sys.argv[2:])
        test_results[test_file] = test_result

        sys.stdout.write(test_result[0]) # print status
        sys.stdout.flush()

    num_failed = sum(1 for res in test_results.values() if res[0] != '.')

    print("")
    if num_failed != 0:
        print("\nFailures:")
        for res in test_results.values():
            if res[0] != '.':
                sys.stdout.write(res[0] + " " + res[2] + "\n")
                sys.stdout.flush()

    sys.stdout.flush()
    return num_failed == 0



def main():
    print("")
    print("Boomerang Regression Tester")
    print("===========================")
    print("")

    # ${CMAKE_BINARY_DIR}/tests/regression-tests
    base_dir = os.getcwd()
    tests_input_base = os.path.abspath(os.path.join(os.getcwd(), "../../out/share/boomerang/samples/"))

    all_ok = True

    clean_old_outputs(base_dir)
    all_ok &= perform_regression_tests(base_dir, tests_input_base, regression_tests)
    all_ok &= perform_smoke_tests(base_dir, tests_input_base, smoke_tests)

    print("Testing finished.\n")

    sys.exit(not all_ok) # Return with 0 exit status if everything is OK



if __name__ == "__main__":
    main()
