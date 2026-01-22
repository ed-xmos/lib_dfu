# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest


def pytest_addoption(parser):
    parser.addoption("--level", action="store", default="default", help="smoke or extended")


@pytest.fixture
def level(request):
    return request.config.getoption("--level")
