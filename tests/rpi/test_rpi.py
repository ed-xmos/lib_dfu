
import pytest
import os
import pathlib
import subprocess

def parse_descriptor(line):
    device_value = ""
    attribute = ""
    mode = ""
    for token in line.split(" "):
        token = token.strip(",")
        if token.startswith("0x"):
            if not device_value:
                device_value = token
            elif not attribute:
                attribute = token
            elif not mode:
                mode = token

    return (device_value, attribute, mode)


def test_rpi():
    factory_device = "0x0101"
    upgrade_device = "0x0200"
    runtime_mode = "0x001"
    dfu_mode = "0x02"

    # Check for dfu_utility
    host_file_path =  pathlib.Path(__file__).parent / "../../host/dfu_i2c/bin/dfu_i2c"
    assert host_file_path.exists(), f"Host file path {host_file_path} does not exist"

    # Check that the test file exists
    suffix_file_path = pathlib.Path(__file__).parent / "../../host/suffix_generator/bin/dfu_suffix_generator"
    assert suffix_file_path.exists(), f"Test file {suffix_file_path} does not exist."

    # Check that the test file exists
    test_file_path = pathlib.Path(__file__).parent / "i2c_update.bin"
    assert test_file_path.exists(), f"Test file {test_file_path} does not exist."

    # Check that the test file is not empty
    assert test_file_path.stat().st_size > 0, f"Test file {test_file_path} is empty."

    # If we reach this point, the test file exists and is not empty
    print(f"Test file {test_file_path} exists and is not empty.")

    target_dfu_file = "i2c_update.dfu"

    subprocess.check_call(f"{suffix_file_path} 0x20b1 0x1234 {test_file_path} {target_dfu_file}".split(), text=True)

    # Test #1
    proc = subprocess.run(f"{host_file_path} detach_and_bus_reset".split(), text=True, capture_output=True)
    print(proc.stdout)
    assert proc.returncode == 0, "Host DFU app failed comms"

#    print(len(proc.stdout))

    lines = proc.stdout.splitlines()
    (fw_value, fw_attr, fw_mode) = parse_descriptor(lines[0])
    (fw_dfu_value, fw_dfu_attr, fw_dfu_mode) = parse_descriptor(lines[-1])
    print(f"desc: {fw_value}, {fw_attr}, {fw_mode}")
    print(f"desc: {fw_dfu_value}, {fw_dfu_attr}, {fw_dfu_mode}")

    if runtime_mode in fw_mode:
        print(f"Device started in runtime mode {fw_mode}")
        if dfu_mode in [fw_dfu_mode, fw_mode]:
            print("Device transition to DFU mode")
    elif dfu_mode in fw_dfu_mode:
        print("Device already in DFU mode")

    assert factory_device in fw_dfu_value, f"Unexpected device value, expected factory ({factory_device}), got {fw_dfu_value}"

    proc = subprocess.run(f"{host_file_path} write_upgrade {test_file_path}".split(), text=True, capture_output=True)
    assert proc.returncode

    # Test #2
    # TODO - run upgrade, detech, check value == dfu_device

    # Test #3
    # TODO - run upload, check file == i2c_update.dfu

    # Test #4
    # TODO - run revert, detach, check value == facotry_device

