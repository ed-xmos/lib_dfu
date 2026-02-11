# Copyright 2020-2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import os
import subprocess


def slots():
    home = os.path.dirname(os.path.abspath(__file__))
    boot_address = 4096
    data_address = 8192
    for (partition, expected) in [(1, boot_address), (2, data_address)]:
        try:
            cmd = ['xsim', '--args', os.path.join(home, 'bin', 'test_slots', 'test_dfu_test_slots.xe')]
            _ = subprocess.check_call(cmd)
        except subprocess.CalledProcessError as e:
            msg = '''Error! Simulator failed
                   \ncmd: %s
                   \noutput: %s
                   \nreturn_code: %d'''\
                   % (str(e.cmd), e.output, e.returncode)
            raise Exception(msg)
