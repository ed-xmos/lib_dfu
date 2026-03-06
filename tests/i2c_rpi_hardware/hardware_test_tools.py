# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XCORE VocalFusion Licence.

import json
from pathlib import Path
from typing import Collection, Union
from xtagctl import XtagctlDeviceNotConnected

class RPiController:
    def __init__(self, conn, binary_path):
        self.conn = conn
        self.binary_path = binary_path

    def run(self, args="", hide=False):
        cmd = f"{self.binary_path} {args}"
        result = self.conn.run(cmd, hide=hide, warn=True, in_stream=False)
        return result


def load_test_settings(adapter_ids):
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

