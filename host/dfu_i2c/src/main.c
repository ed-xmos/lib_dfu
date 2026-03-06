// Copyright 2016-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "argument_parser.h"
#include "input_reader.h"
#include "device_id.h"
#include "operations.h"
#include "hal.h"
#include "dfu_utils.h"

int main(int argc, char **argv)
{
  struct options options = parse_arguments(argc, argv);
  int ret = 0;

  switch (options.operation) {
    case WRITE_UPGRADE: {
      struct inputs inputs = read_write_upgrade_inputs(options.arguments[0], options.device_id);

      // block number is 16 bits with top bit reserved for boot/data marker
      // so maximum block count is 32,768
      const size_t fifteen_bits_max = 32768;
      const size_t max_dnload_size = fifteen_bits_max * options.block_size;

      if (inputs.boot.length > max_dnload_size) {
        PRINT_ERROR("Boot image size %lu exceeds maximum %lu\n", inputs.boot.length, max_dnload_size);
        return 1;
      }

      if (hal_connect(options.device_id) != 0) {
        return 1;
      }

      ret = write_upgrade(inputs, options.block_size);

      if (ret == 0) {
        hal_reboot();
      }

      hal_disconnect();
      cleanup_inputs(&inputs);
      break;
    }

    case UPLOAD: {
      printf("Uploading\n");

      if (hal_connect(options.device_id) != 0) {
        return 1;
      }

      unsigned char *buffer;
      int size;
      ret = read_upload(buffer, size, options.block_size);

      hal_disconnect();

      ret = write_upload_file_to_disk();
      if (ret != 0) {
        printf("write binary upload file to disk failed\n");
      }
      break;
    }

    case DETACH_AND_BUS_RESET: {
      if (hal_connect(options.device_id) != 0) {
        return 1;
      }

      ret = detach_and_bus_reset();

      hal_disconnect();
      break;
    }

    case REBOOT: {
      if (hal_connect(options.device_id) != 0) {
        printf("Connect failed\n");
        return 1;
      }

      (void)hal_reboot();
      printf("Device rebooted\n");

      hal_disconnect();
      break;
    }

    case REVERT_FACTORY: {
      printf("Revert factory\n");
      if (hal_connect(options.device_id) != 0) {
        printf("Connect failed\n");
        return 1;
      }

      ret = detach_and_bus_reset();
      if (ret != 0) {
        hal_disconnect();
        return ret;
      }
      ret = hal_revert_factory();
      if (ret != 0) {
        printf("Revert factory failed\n");
      }
      (void)hal_reboot();
      
      printf("Device rebooted\n");
      
      hal_disconnect();
      break;
    }

    default:
      assert(0);
      break;
  }

  if (ret != 0) {
    return 1;
  }

  return 0;
}
