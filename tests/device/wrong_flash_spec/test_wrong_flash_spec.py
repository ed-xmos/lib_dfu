# Copyright (c) 2019, XMOS Ltd, All rights reserved
import subprocess, os

def test_wrong_flash_spec():
    home = os.path.dirname(os.path.abspath(__file__))
    try:
        cmd = ['xsim', os.path.join(home, 'bin/wrong_flash_spec.xe')]
        output = subprocess.check_call(cmd)
    except subprocess.CalledProcessError as e:
        msg = '''Error! Simulator failed
               \ncmd: %s
               \noutput: %s
               \nreturn_code: %d'''\
               % (str(e.cmd), e.output, e.returncode)
        raise Exception(msg)

if __name__ == "__main__":
    print('test_wrong_flash_spec')
    test_wrong_flash_spec()
