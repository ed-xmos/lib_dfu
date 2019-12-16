# Copyright (c) 2019, XMOS Ltd, All rights reserved
import subprocess, os

def test_dnload():
    home = os.path.dirname(os.path.abspath(__file__))
    test_instances = [
        (32, 1), (32, 3), (32, 8), (32, 9), (32, 128), (32, 1024),
        (128, 1), (128, 2), (128, 8), (128, 9), (128, 128), (128, 256),
        (256, 1), (256, 2), (256, 8), (256, 16), (256, 17), (256, 128),
        (512, 1), (512, 7), (512, 8), (512, 9), (512, 32),
    ]
    for (block_size, block_count) in test_instances:
        try:
            cmd = ['axe', '--args', os.path.join(home, 'bin/dnload.xe'),
                   str(block_size), str(block_count)]
            subprocess.check_call(cmd)
        except subprocess.CalledProcessError as e:
            msg = '''Error! Simulator failed
                   \ncmd: %s
                   \noutput: %s
                   \nreturn_code: %d'''\
                   % (str(e.cmd), e.output, e.returncode)
            raise Exception(msg)

if __name__ == "__main__":
    test_dnload()
