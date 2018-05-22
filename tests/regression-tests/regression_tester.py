#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys
import difflib

from collections import defaultdict
from filecmp import dircmp


# These files are used for checking for regressions
regression_tests = [
    "elf/hello-clang4-dynamic",
    "pentium/asgngoto",
    "pentium/banner",
    "pentium/branch",
    "pentium/branch-linux",
    "pentium/bswap",
    "pentium/callchain",
    "pentium/chararray",
    "pentium/chararray-O4",
    "pentium/daysofxmas",
    "pentium/encrypt",
    "pentium/fbranch",
    "pentium/fbranch2",
    "pentium/fedora2_true",
    "pentium/fedora3_true",
    "pentium/fib",
    "pentium/fibo2",
    "pentium/fibo3",
    "pentium/fibo4",
    "pentium/fibo_iter",
    "pentium/fibo-O4",
    "pentium/fromssa2",
    "pentium/frontier",
    "pentium/funcptr",
    "pentium/global1",
    "pentium/hello",
    "pentium/ifthen",
    "pentium/line1",
    "pentium/line1-o4",
    "pentium/localarray",
    "pentium/localarray-O4",
    "pentium/loop",
    "pentium/manyparams",
    "pentium/minmax",
    "pentium/minmax2",
    "pentium/minmax3",
    "pentium/nestedswitch",
    "pentium/param1",
    "pentium/paramchain",
    "pentium/phi",
    "pentium/phi2",
    "pentium/printpi",
    "pentium/recursion",
    "pentium/regalias",
    "pentium/regalias2",
    "pentium/restoredparam",
    "pentium/rux_encrypt",
    "pentium/semi",
    "pentium/set",
    "pentium/shared2",
    "pentium/short1",
    "pentium/short2",
    "pentium/stattest",
    "pentium/sumarray",
    "pentium/sumarray-O4",
    "pentium/superstat",
    "pentium/suse_true",
    "pentium/switch_cc",
    "pentium/switch_gcc",
    "pentium/testarray1",
    "pentium/testarray2",
    "pentium/testset",
    "pentium/twofib",
    "pentium/twoproc",
    "pentium/twoproc2",
    "pentium/twoproc3",
    "pentium/uns"
]

# These files are currently disabled or unused
disabled_tests = [
    "pentium/ass2",
    "pentium/ass3",
    "pentium/global2",
    "pentium/global3",
    "pentium/recursion2",
]



""" Clean output directories from old data. """
def clean_old_outputs(base_dir):
    print("Cleaning up old data ...")
    output_dir = os.path.join(base_dir, "outputs")
    if os.path.isdir(output_dir): shutil.rmtree(output_dir)
    os.makedirs(output_dir)



""" Compare directories and print the differences of file content. Returns True if the directories are equal. """
def compare_directories(dir_left, dir_right):
    def compare_directories_internal(dcmp):
        directories_equal = True
        for different_file_name in dcmp.diff_files:
            # Found different file
            directories_equal = False

            with open(os.path.join(dcmp.left,  different_file_name), 'r') as file_left, \
                 open(os.path.join(dcmp.right, different_file_name), 'r') as file_right:
                diff = difflib.unified_diff(file_left.readlines(), file_right.readlines(),
                    fromfile="%s (expected)" % different_file_name,
                    tofile  ="%s (actual)"   % different_file_name)

                print("")
                for line in diff:
                    sys.stdout.write(line)
                print("")

        for sub_dcmp in dcmp.subdirs.values():
            directories_equal &= compare_directories_internal(sub_dcmp)

        return directories_equal

    dcmp = dircmp(dir_left, dir_right)
    return compare_directories_internal(dcmp)


""" Perform the actual test on a single input binary """
def test_single_input(cli_path, input_file, output_path, desired_output_path, args):
    cmdline   = [cli_path] + ['-P', os.path.dirname(cli_path), '-o', output_path] + args + [input_file]

    try:
        with open(os.path.join(output_path, os.path.basename(input_file) + ".stdout"), "w") as test_stdout, \
             open(os.path.join(output_path, os.path.basename(input_file) + ".stderr"), "w") as test_stderr:

            try:
                result = subprocess.call(cmdline, stdout=test_stdout, stderr=test_stderr, timeout=120)
                result = '.' if result == 0 else 'f'

                if result == '.':
                    # Perform regression diff
                    if not compare_directories(desired_output_path, output_path):
                        result = 'r'
            except:
                result = '!'

        return [result, ' '.join(cmdline), input_file]
    except:
        return ['d', ' '.join(cmdline), input_file]



""" Perform regression tests on inputs in test_list. Returns true on success (no regressions). """
def perform_regression_tests(base_dir, test_input_base, test_list):
    test_results = defaultdict();

    sys.stdout.write("Testing for regressions ")
    for test_file in test_list:
        input_file = os.path.join(test_input_base, test_file)
        desired_output_dir = os.path.join(base_dir, "desired-outputs", test_file)
        output_dir = os.path.join(base_dir, "outputs", test_file)
        os.makedirs(output_dir)

        test_result = test_single_input(sys.argv[1], input_file, output_dir, desired_output_dir, sys.argv[2:])
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

    sys.stdout.flush()
    return num_failed == 0


def main():
    print("")
    print("Boomerang 0.4.0 Regression Tester")
    print("=================================")
    print("")

    # ${CMAKE_BINARY_DIR}/tests/regression-tests
    base_dir = os.getcwd()
    tests_input_base = os.path.abspath(os.path.join(os.getcwd(), "../../out/share/boomerang/samples/"))

    all_ok = True

    clean_old_outputs(base_dir)
    all_ok &= perform_regression_tests(base_dir, tests_input_base, regression_tests)

    print("Testing finished.\n")

    sys.exit(not all_ok) # Return with 0 exit status if everything is OK


if __name__ == "__main__":
    main()
