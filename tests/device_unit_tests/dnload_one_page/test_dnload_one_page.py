# Copyright (c) 2019, XMOS Ltd, All rights reserved
import subprocess, os

def test_dnload_one_page():
    home = os.path.dirname(os.path.abspath(__file__))
    try:
        cmd = ['xsim', os.path.join(home, 'bin/dnload_one_page.xe')]
        output = subprocess.check_output(cmd)
    except subprocess.CalledProcessError as e:
        msg = '''Error! Simulator failed
               \ncmd: %s
               \noutput: %s
               \nreturn_code: %d'''\
               % (str(e.cmd), e.output, e.returncode)
        raise Exception(msg)
    print(output)
