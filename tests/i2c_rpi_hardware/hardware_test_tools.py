# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import json
from pathlib import Path


class RPiController:
    def __init__(self, conn, binary_dfu, binary_suffix_generator, REMOTE_DIR):
        self.conn = conn
        self.binary_dfu = binary_dfu
        self.binary_suffix_generator = binary_suffix_generator
        self.REMOTE_DIR = REMOTE_DIR

    def run_dfu(self, args="", hide=False):
        cmd = f"{self.binary_dfu} {args}"
        result = self.conn.run(cmd, hide=hide, warn=True, in_stream=False)
        return result

    def run_suffix_generator(self, args="", hide=False):
        cmd = f"{self.binary_suffix_generator} {args}"
        result = self.conn.run(cmd, hide=hide, warn=True, in_stream=False)
        return result

    def send_file(self, local_path):
        remote_path = str(Path(self.REMOTE_DIR) / Path(local_path).name)
        self.conn.put(local_path, remote_path)


def load_test_settings(adapter_ids):
    """
    Loads test settings from a local JSON file if it exists, otherwise uses defaults suitable for running in XMOS CI
    To run locally, create a file at tests/i2c_rpi_hardware/local_test_settings.json with the following format:

    {
        "test settings": [
            {
                "adapter_id": "xxxxxxx",
                "comment": "Adapter ID of the xTAG connected to the device, if not defined xtagctl is used. Default value is empty string"
            },
            {
                "pi_server_ip": "192.168.1.xxx",
                "comment": "IP address of the Raspberry Pi connected to the device. Default is empty"
            },
            {
                "pi_server_login": "mylogin",
                "comment": "Login username for the Raspberry Pi connected to the device. Default is empty"
            },
            {
                "pi_server_password": "mypassword",
                "comment": "Login password for the Raspberry Pi connected to the device. Default is empty"
            }
        ]
    }
    """
    settings_path = Path(__file__).parent / 'local_test_settings.json'
    settings = {}
    if settings_path.is_file():
        f = open(settings_path)
        data = json.load(f)
        f.close()
        settings["adapter_id"] = adapter_ids[0] if adapter_ids else data["test settings"][0]["adapter_id"]
        settings["pi_server_ip"] = data["test settings"][1]["pi_server_ip"]
        settings["pi_server_login"] = data["test settings"][2]["pi_server_login"]
        settings["pi_server_password"] = data["test settings"][3]["pi_server_password"]

        print(
            f"Using local test settings: {json.dumps(settings, sort_keys=True, indent=4)}"
        )
    else:
        print(adapter_ids, adapter_ids[0])
        settings["adapter_id"] = adapter_ids[0]
        settings["pi_server_ip"] = adapter_ids[0] # There is a DNS entry for the RPi with the same name as the XTAG ID
        settings["pi_server_login"] = "pi"
        settings["pi_server_password"] = None # We have local SSH keys

    return settings

