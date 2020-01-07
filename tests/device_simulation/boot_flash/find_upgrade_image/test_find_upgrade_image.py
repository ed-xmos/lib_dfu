# Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
import subprocess, os

def test_find_upgrade_image():
    home = os.path.dirname(os.path.abspath(__file__))
    try:
        cmd = ['xsim', os.path.join(home, 'bin', 'find_upgrade_image.xe')]
        output = subprocess.check_call(cmd)
    except subprocess.CalledProcessError as e:
        msg = '''Error! Simulator failed
               \ncmd: %s
               \noutput: %s
               \nreturn_code: %d'''\
               % (str(e.cmd), e.output, e.returncode)
        raise Exception(msg)

if __name__ == "__main__":
    print('test_find_upgrade_image')
    test_find_upgrade_image()
