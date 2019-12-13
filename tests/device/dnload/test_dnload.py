# Copyright (c) 2019, XMOS Ltd, All rights reserved
import subprocess, os

def test_dnload():
    home = os.path.dirname(os.path.abspath(__file__))
    # 8 blocks to 1 256-byte page, 128 blocks to one 4KB sector (16 pages)
    for block_count in [1, 8, 128, 1024]:
        try:
            cmd = ['axe', '--args', os.path.join(home, 'bin/dnload.xe'), str(block_count)]
            output = subprocess.check_output(cmd)
        except subprocess.CalledProcessError as e:
            msg = '''Error! Simulator failed
                   \ncmd: %s
                   \noutput: %s
                   \nreturn_code: %d'''\
                   % (str(e.cmd), e.output, e.returncode)
            raise Exception(msg)
        print(output)
