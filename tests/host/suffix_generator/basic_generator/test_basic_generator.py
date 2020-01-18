# Copyright (c) 2020, XMOS Ltd, All rights reserved
import subprocess, os

def test_basic_generator():
    home = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(home, 'input.bin'), 'rb') as in_file:
        cmd = [os.path.join(home, 'bin', 'suffix_generator'),
               '0x21B1', '0x0014', '0x0210']
        try:
            output = subprocess.check_output(cmd, stdin=in_file)
        except subprocess.CalledProcessError as e:
            msg = '''Error! Test failed
                   \ncmd: %s
                   \nreturn_code: %d'''\
                   % (' '.join(e.cmd), e.returncode)
            raise Exception(msg)
        with open(os.path.join(home, 'golden.bin'), 'rb') as golden_file:
            expected = golden_file.read()
            if output != expected:
                raise Exception('''Error! Test failed
                      \ncommand line: %s'''\
                      % ' '.join(cmd))
        print('PASS')

if __name__ == "__main__":
    print('test_basic_generator')
    test_basic_generator()
