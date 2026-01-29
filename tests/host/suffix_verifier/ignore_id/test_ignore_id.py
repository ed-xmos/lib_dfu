# Copyright 2019-2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import os
import subprocess


def test_ignore_id():
    home = os.path.dirname(os.path.abspath(__file__))
    try:
        cmd = [os.path.join(home, 'bin/ignore_id')]
        _ = subprocess.check_call(cmd)
    except subprocess.CalledProcessError as e:
        msg = '''Error! Test failed
               \ncmd: %s
               \noutput: %s
               \nreturn_code: %d'''\
               % (' '.join(e.cmd), e.output, e.returncode)
        raise Exception(msg)
