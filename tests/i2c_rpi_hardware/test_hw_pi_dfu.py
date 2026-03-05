# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.


import pytest
from pathlib import Path
import zipfile
from fabric import Connection
from hardware_test_tools import load_local_settings



class RPiController:
    def __init__(self, conn, binary_path):
        self.conn = conn
        self.binary_path = binary_path

    def run(self, args="", hide=True):
        cmd = f"{self.binary_path} {args}"
        result = self.conn.run(cmd, hide=hide, warn=True, in_stream=False)
        return result


@pytest.fixture(scope="session")
def dfu_app_rpi(settings):
    conn = Connection(settings["pi_server_ip"], connect_kwargs={'password': settings["pi_server_password"]})
    REMOTE_DIR = "/tmp/pytest_rpi_build"

    repo_root = Path(__file__).resolve().parent.parent.parent.parent  # new_afenext/
    lib_dfu_dir = repo_root / "lib_dfu"
    lib_device_control_dir = repo_root / "lib_device_control"

    # zip both repos into one archive, preserving their directory names
    print("Zipping host code...")
    EXCLUDE = {
        "lib_dfu":           {"doc", "examples", "tests"},
        "lib_device_control": {"doc", "examples", "tests", "python", "tools"},
    }

    zip_path = "/tmp/host_upload.zip"
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for repo_dir in (lib_dfu_dir, lib_device_control_dir):
            exclude_dirs = EXCLUDE[repo_dir.name]
            for file in repo_dir.rglob("*"):
                if file.is_file():
                    rel = file.relative_to(repo_dir)
                    if rel.parts[0] not in exclude_dirs:
                        zf.write(file, file.relative_to(repo_root))

    # clean remote workspace, upload zip, and unzip
    print("Uploading host code to RPi...")
    conn.run(f"rm -rf {REMOTE_DIR} && mkdir -p {REMOTE_DIR}", in_stream=False)
    conn.put(zip_path, f"{REMOTE_DIR}/host_upload.zip")
    print("Unzipping host code on RPi...")
    conn.run(f"cd {REMOTE_DIR} && unzip -o host_upload.zip", in_stream=False, hide=True)

    # configure + build
    print("Configuring and building host code on RPi...")
    conn.run(f"""
        cd {REMOTE_DIR} &&
        cd lib_dfu/host/dfu_i2c/ &&
        cmake -B build &&
        cmake --build build -j 4
    """, in_stream=False, hide=True)

    binary = f"{REMOTE_DIR}/lib_dfu/host/dfu_i2c/bin/dfu_i2c"
    print("RPi ready!")

    yield RPiController(conn, binary)

    # cleanup
    conn.run(f"rm -rf {REMOTE_DIR}", in_stream=False)



"""
"""


def test_dfu_commands(dfu_app_rpi):
    result = dfu_app_rpi.run("--help")
    assert result.return_code == 2  # --help exits with 2 by design
    print(result.stdout + result.stderr)
    print("PASS")