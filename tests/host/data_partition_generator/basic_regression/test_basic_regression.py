# Copyright (c) 2019, XMOS Ltd, All rights reserved
import subprocess, os, tempfile, shutil, filecmp

def test_basic_regression():
    home = os.path.dirname(os.path.abspath(__file__))
    tmp_dir = tempfile.mkdtemp()
    tmp_file = os.path.join(tmp_dir, 'data.bin')
    factory_json = os.path.join(home, 'factory.json')
    upgrade_json = os.path.join(home, 'upgrade.json')
    test_instances = [
        ['--factory', factory_json],
        ['--factory', factory_json, '--upgrade', '514', upgrade_json],
        ['--bad-factory-crc', '--factory', factory_json],
        ['--bad-upgrade-crc', '--factory', factory_json, '--upgrade', '514', upgrade_json]
    ]
    n = 1
    for instance in test_instances:
        print(n)
        try:
            cmd = [os.path.join(home, 'data_partition_generator'),
                   '--regular-sector-size', '4096',
                   '-o', tmp_file] + instance
            output = subprocess.check_call(cmd)
        except subprocess.CalledProcessError as e:
            msg = '''Error! Test failed
                   \ncmd: %s
                   \noutput: %s
                   \nreturn_code: %d'''\
                   % (str(e.cmd), e.output, e.returncode)
            shutil.rmtree(tmp_dir)
            raise Exception(msg)
        golden_file = 'golden%d.bin' % n
        if not filecmp.cmp(tmp_file, os.path.join(home, golden_file)):
            raise Exception('''Error! Test failed
                  \ntest instance: %s
                  \ngolden file: %s'''\
                  % (' '.join(instance), golden_file))
        n += 1
        print('PASS')
    shutil.rmtree(tmp_dir)

if __name__ == "__main__":
    print('test_basic_regression')
    test_basic_regression()
