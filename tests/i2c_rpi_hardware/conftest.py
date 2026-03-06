# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re
import zipfile
import pytest
from pathlib import Path
from fabric import Connection
from hardware_test_tools import load_test_settings, RPiController


def pytest_addoption(parser):
    parser.addoption("--adapter-id", action="store", default=None, help="XTAG adapter ID(s) - optional")


def parse_adapter_ids(config):
    """
    returns a list of adapter IDs, or None if not provided.
    If not None, it always returns a list, even if only one ID is provided.
    """
    xtag_ids = config.getoption("--adapter-id", default=None)
    if xtag_ids is None:
        return None
    s = xtag_ids.strip()
    # Case 1: Already a single value, no colons or brackets
    if ":" not in s and "[" not in s and "]" not in s:
        return [s]
    # Case 2: Structured form like [0:XTAG0, 1:XTAG2]
    ids = re.findall(r':\s*([^,\]]+)', s)
    return ids


@pytest.fixture(scope="session")
def adapter_ids(request):
    return parse_adapter_ids(request.config)


@pytest.fixture(scope="session")
def settings(adapter_ids):
    return load_test_settings(adapter_ids)


@pytest.fixture(scope="session")
def dfu_app_rpi(settings):
    connect_kwargs = {}
    password = settings.get("pi_server_password")
    if password:
        connect_kwargs["password"] = password

    conn = Connection(settings["pi_server_ip"],
                      user=settings["pi_server_login"],
                      connect_kwargs=connect_kwargs)
    REMOTE_DIR = "/tmp/pytest_rpi_build"

    repo_root = Path(__file__).resolve().parent.parent.parent.parent  # new_afenext/
    lib_dfu_dir = repo_root / "lib_dfu"
    lib_device_control_dir = repo_root / "lib_device_control"

    EXCLUDE = {
        "lib_dfu":            {"doc", "examples", "tests"},
        "lib_device_control": {"doc", "examples", "tests", "python", "tools"},
    }

    print("Zipping host code...")
    zip_path = "/tmp/host_upload.zip"
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for repo_dir in (lib_dfu_dir, lib_device_control_dir):
            exclude_dirs = EXCLUDE[repo_dir.name]
            for file in repo_dir.rglob("*"):
                if file.is_file():
                    rel = file.relative_to(repo_dir)
                    if rel.parts[0] not in exclude_dirs:
                        zf.write(file, file.relative_to(repo_root))

    print("Uploading host code to RPi...")
    conn.run(f"rm -rf {REMOTE_DIR} && mkdir -p {REMOTE_DIR}", in_stream=False)
    conn.put(zip_path, f"{REMOTE_DIR}/host_upload.zip")
    print("Unzipping host code on RPi...")
    conn.run(f"cd {REMOTE_DIR} && unzip -o host_upload.zip", in_stream=False, hide=True)

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

    conn.run(f"rm -rf {REMOTE_DIR}", in_stream=False)
