# Copyright (c) 2020, XMOS Ltd, All rights reserved
import subprocess, os

def test_page_size():
    home = os.path.dirname(os.path.abspath(__file__))
    for (page_size, good) in [(0, False), (128, True), (256, True), (512, False)]:
        try:
            cmd = ['xsim', '--args', os.path.join(home, 'bin', 'page_size.xe'),
                   str(page_size), str(int(good))]
            output = subprocess.check_call(cmd)
        except subprocess.CalledProcessError as e:
            msg = '''Error! Simulator failed
                   \ncmd: %s
                   \noutput: %s
                   \nreturn_code: %d'''\
                   % (str(e.cmd), e.output, e.returncode)
            raise Exception(msg)

if __name__ == "__main__":
    print('test_page_size')
    test_page_size()
