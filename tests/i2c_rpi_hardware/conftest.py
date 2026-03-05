# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re
import pytest
from hardware_test_tools import load_local_settings


def pytest_addoption(parser):
    parser.addoption("--adapter-id", action="store", default=None, help="XTAG adapter ID(s) - optional")


def parse_adapter_ids(config):
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
    return load_local_settings(adapter_ids, target=None)
