# Copyright 2020-2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import os
import subprocess
import pathlib

HOME = str(pathlib.Path(__file__).resolve().parent)


def scenario(*argv):
    cmd = ['../../../../host/suffix_generator/bin/dfu_suffix_generator'] + list(argv)
    print('- %s' % ' '.join(argv))
    try:
        _ = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError:
        return
    else:
        raise Exception('unexpected success: %s' % ' '.join(argv))


def test_incorrect_usage():
    os.chdir(HOME)

    # file not found
    scenario('0x20B1', '0x0014', 'input.bin', 'output.bin')

    # wrong number of arguments
    scenario()
    scenario('output.bin')
    scenario('input.bin', 'output.bin')
    scenario('0x0102', 'input.bin', 'output.bin')
    scenario('foobar', '0x20B1', '0x0014', '0x0102', 'input.bin', 'output.bin')

    # not a number
    scenario('spice', '0x0014', '0x0102', 'input.bin', 'output.bin')
    scenario('0x20B1', 'must', '0x0102', 'input.bin', 'output.bin')
    scenario('0x20B1', '0x0014', 'flow', 'input.bin', 'output.bin')

    print('PASS')
