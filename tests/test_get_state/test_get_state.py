# Copyright (c) 2019, XMOS Ltd, All rights reserved
import os, subprocess, tempfile

def test_get_state():
    tmpfile_out = tempfile.mktemp('.bin')
    try:
        cmd = ['xsim', '--args', 'bin/test_get_state.xe', tmpfile_out] # TODO use axe
        output = subprocess.check_output(cmd)
    except subprocess.CalledProcessError as e:
        msg = '''Error! Simulator failed
               \ncmd: %s
               \noutput: %s
               \nreturn_code: %d'''\
               % (str(e.cmd), e.output, e.returncode)
        raise Exception(msg)
    print(output)
    try:
        os.unlink(tmpfile_out)
    except FileNotFoundError:
        pass
