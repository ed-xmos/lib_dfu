# Copyright 2020-2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import os
import subprocess
import filecmp
import pathlib

HOME = str(pathlib.Path(__file__).resolve().parent)


def test_without_bcddevice():
    os.chdir(HOME)
    cmd = ['../../../../host/suffix_generator/bin/dfu_suffix_generator',
           '0x20B1', '0x0014', 'input.bin', 'output.bin']
    try:
        _ = subprocess.check_output(cmd)
    except subprocess.CalledProcessError as e:
        msg = '''Error! Test failed
                \ncmd: %s
                \nreturn_code: %d'''\
                % (' '.join(e.cmd), e.returncode)
        raise Exception(msg)
    if not filecmp.cmp(os.path.join(HOME, 'output.bin'),
                       os.path.join(HOME, 'golden.bin')):
        raise Exception('''Error! Test failed
                        \ncommand line: %s'''
                        % ' '.join(cmd))
    print('PASS')
