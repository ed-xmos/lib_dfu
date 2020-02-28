# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
import subprocess, os

def test_basic_verifier():
    home = os.path.dirname(os.path.abspath(__file__))
    try:
        cmd = [os.path.join(home, os.path.join('bin', 'basic_verifier'))]
        output = subprocess.check_call(cmd)
    except subprocess.CalledProcessError as e:
        msg = '''Error! Test failed
               \ncmd: %s
               \noutput: %s
               \nreturn_code: %d'''\
               % (' '.join(e.cmd), e.output, e.returncode)
        raise Exception(msg)

if __name__ == "__main__":
    print('test_basic_verifier')
    test_basic_verifier()
