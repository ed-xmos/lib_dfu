# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.


def test_dfu_commands(dfu_app_rpi):
    """Test that the DFU app on the Raspberry Pi can be run and responds to --help."""

    result = dfu_app_rpi.run("--help")
    assert result.return_code == 2  # --help exits with 2 by design
    print(result.stdout + result.stderr)
    print("PASS")
    