# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import subprocess
import re
from pathlib import Path


def pytest_addoption(parser):
    parser.addoption("--level", action="store", default="smoke", help="smoke or extended")
    parser.addoption("--adapter-id", action="store", default="", help="XTAG adapter ID")


# Is there a better way to pass cmd=line options to test that inherit from pytest.Item? I couldn't find a way to do
# this without using global variables, which is not ideal but seems to be the only way.
level = None
adapter_id = None


def pytest_configure(config):
    subprocess.run(["cmake", "-B", "build"], check=True)
    subprocess.run(["cmake", "--build", "build"], check=True)

    global level, adapter_id
    level = config.getoption("--level")
    adapter_id = config.getoption("--adapter-id")

    if adapter_id:
        # This is required for the prepare_upgrade_slot tests
        subprocess.run(["xflash", "--adapter-id", adapter_id, "--factory", "../../dummy/bin/hello_world.xe"], check=True)
    else:
        # This is required for the prepare_upgrade_slot tests
        subprocess.run(["xflash", "--factory", "../../dummy/bin/hello_world.xe"], check=True)

    subprocess.run(["xflash", "--factory-version", "15.3", "--upgrade", "1", "../../dummy/bin/hello_world.xe",
                    "-o", "../../dummy/bin/hello_world.bin"], check=True)


def pytest_collect_file(parent, file_path: Path):
    """Custom collection function to inform pytest that xe files contain tests."""
    if file_path.suffix == ".xe":
        return UnityTestSource.from_parent(parent, path=file_path)


class UnityTestSource(pytest.File):
    """
    Each xe file contains 1 pytest test.
    """

    def collect(self):
        yield UnityTestExecutable.from_parent(self, xe=self.path, name=self.path.stem, level=level, adapter_id=adapter_id)


class UnityTestExecutable(pytest.Item):
    """
    Run the xe file in xsim, this is the work of the test.
    """

    def __init__(self, xe, level, adapter_id, **kwargs):
        super().__init__(**kwargs)
        self.xe = xe
        self.fail_reason = []
        self.level = level
        self.adapter_id = adapter_id

    def runtest(self):
        """
        fancy test output processing.
        """
        # Test time limits for given operations
        erase_time_ms = "15000"
        write_time_ms = "200"
        xrun_cmd = ["xrun", "--xscope"]
        if self.adapter_id:
            xrun_cmd += ["--adapter-id", self.adapter_id]
        xrun_cmd += ["--args", self.xe, "../../dummy/bin/hello_world.bin", erase_time_ms, write_time_ms]
        proc = subprocess.run(xrun_cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        self.add_report_section("call", "stdout", proc.stdout)
        unity_result_pattern = r"^(?P<path>[^\n:]+):(?P<line>\d+):(?P<name>[^:]+):(?P<status>PASS|FAIL)(: (?P<message>.*))?$"
        unlikely_repl = "unlikely_repl"

        result = [i for i in re.finditer(unity_result_pattern, proc.stdout, re.MULTILINE)]
        all_out = [i for i in re.sub(unity_result_pattern, unlikely_repl, proc.stdout,
                                     flags=re.MULTILINE).split(unlikely_repl)]

        for match, output in zip(result, all_out):
            file, line, test_name, status, message = match.group("path", "line", "name", "status", "message")
            fail_reason = f"{test_name}:{line}:{message}"
            self.add_report_section("call", f"{status} {test_name}", output + "\n" + fail_reason)
            if status == "FAIL":
                self.fail_reason.append(fail_reason)
        if proc.returncode:
            raise UnityTestException

    def repr_failure(self, excinfo):
        if isinstance(excinfo.value, UnityTestException):
            return "Failure summary:\n\t" + "\n\t".join(self.fail_reason)
        return super().repr_failure(excinfo)

    def reportinfo(self):
        return self.path, 0, self.xe.stem


class UnityTestException(Exception):
    pass
