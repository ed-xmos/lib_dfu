# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XCORE VocalFusion Licence.

import socket
import sys
import json
from pathlib import Path
from typing import Collection, Union
import xtagctl
from xtagctl import XtagctlDeviceNotConnected

"""

To detect the correct Raspberry Pi IP address, we perform a DNS lookup
for the XTAG ID, which returns a CNAME pointing to the RPi controller name.
We first assume the host OS networking DNS search is set correctly and
check using the shortname, this allows the host OS to decide which
domain we are actually on.

New Raspberry Pi and XTAG used in Jenkins must be correctly added by the DevOps team.

If running the tests locally, users can add their own XTAG ID and Raspberry Pi
IP address by modifying the correct items in the local_test_settings.json file.
This file is usually stored in the sw_xvf38xx repo under test/hardware_test/.

Inspired by https://github.com/xmos/fwk_xvf/blob/develop/modules/hardware_test_tools/hardware_test_tools/pi_ip_resolve.py

"""


def resolve_ip_address(device_name, hostname=False):
    """Uses DNS to resolve a short device name to an IP address, appends xmos.local as a fall back"""

    device_ip = None

    try:
        device_ip = socket.gethostbyname_ex(device_name)

    except socket.gaierror as se1:
        print(
            f"{se1} - Device name cannot be resolved on the network using short name: {device_name}",
            file=sys.stderr,
        )

        try:
            device_ip = socket.gethostbyname_ex(device_name + ".xmos.local")

        except socket.gaierror as se2:
            print(
                f"{se2} - Device name cannot be resolved on the network using fqdn: {device_name}.xmos.local\nUsing internal hardware map.",
                file=sys.stderr,
            )

    if hostname:
        return device_ip[0]
    else:
        return device_ip[2][0]


def get_rpi_ip_from_xtag_id(xtag_id):
    """Returns an IP address for the XTAG's relevant connected controller."""

    ip_address = resolve_ip_address(xtag_id)

    assert ip_address, f"Cannot resolve IP address linked to XTAG ID {xtag_id}"
    print(f"Resolved {xtag_id} to address: {ip_address}", file=sys.stdout)
    return ip_address


def get_rpi_hostname_from_xtag_id(xtag_id):
    """Returns a hostname for the XTAG's relevant connected controller."""

    hostname = resolve_ip_address(xtag_id, hostname=True)
    hostname = hostname.split(".")[0]  # HN.xmos.local -> HN

    assert hostname, f"Cannot resolve hostname linked to XTAG ID {xtag_id}"
    print(f"Resolved {xtag_id} to host: {hostname}", file=sys.stdout)
    return hostname


def find_first_specified_xtag(target):
    """
    Get the ID of an xtag from it's reference name. To check available names or add new ones
    look here https://github0.xmos.com/xmos-int/xtagctl_config/blob/master/device_map

    Args:
        target: str or iterable of str, names to try

    Returns:
        The first xtag in the list which corresponds to an available xtag

    Raises:
        RuntimeError: None of the xtags were available
    """
    if isinstance(target, str):
        target = (target,)

    xtag_id = None
    for adapter in target:
        try:
            cm = xtagctl.acquire(adapter)
            with cm as this_id:
                xtag_id = this_id
        except XtagctlDeviceNotConnected:
            continue
        else:
            break

    if xtag_id is None:
        raise RuntimeError("No XTAGs found with specified target names!")

    return xtag_id


def load_local_settings(adapter_ids, target: Union[Collection, str], use_hostname: bool = False):
    settings_path = Path(__file__).parent / 'local_test_settings.json'
    settings = {}
    if False and settings_path.is_file():
        f = open(settings_path)
        data = json.load(f)
        f.close()
        settings["adapter_id"] = adapter_ids[0] if adapter_ids else data["test settings"][0]["adapter_id"]
        settings["pi_server_ip"] = data["test settings"][1]["pi_server_ip"]
        settings["pi_server_login"] = data["test settings"][2]["pi_server_login"]
        settings["pi_server_password"] = data["test settings"][3]["pi_server_password"]

        adapter_id = settings["adapter_id"]

        if not settings["pi_server_ip"] and not is_windows():
            print(adapter_id)
            settings["pi_server_ip"] = (
                get_rpi_ip_from_xtag_id(settings["adapter_id"])
                if not use_hostname
                else get_rpi_hostname_from_xtag_id(settings["adapter_id"])
            )

        print(
            f"Using local test settings: {json.dumps(settings, sort_keys=True, indent=4)}"
        )
    
    else:
        print(adapter_ids, adapter_ids[0])
        settings["adapter_id"] = adapter_ids[0] 
        settings["pi_server_ip"] = get_rpi_ip_from_xtag_id(adapter_ids[0])


    return settings

