// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include "argument_parser.h"
#include "dfu_utils.h"

bool quiet = true;

void print_usage(FILE *stream)
{
  fprintf(stream, "\
usage:      dfu_i2c --help\n\
            dfu_i2c OPTIONS write_upgrade <boot.dfu>\n\
            dfu_i2c OPTIONS detach_and_bus_reset\n\
            dfu_i2c OPTIONS reboot\n\
            dfu_i2c OPTIONS revert_factory\n\
            dfu_i2c OPTIONS upload <file.bin>\n\
\n\
OPTIONS:    --quiet\n\
            --i2c-address 0x%02X (default)\n\
            --block-size %d (default)\n",
          I2C_ADDRESS_DEFAULT,
          BLOCK_SIZE_DEFAULT);
}

const char *operation_str(int operation)
{
  switch (operation) {
    case WRITE_UPGRADE:         return "write_upgrade";
    case DETACH_AND_BUS_RESET:  return "detach_and_bus_reset";
    case REBOOT:                return "reboot";
    case REVERT_FACTORY:        return "revert_factory";
    case UPLOAD:                return "upload";
    default: return "?";
  }
}

int parse_operation(const char *arg)
{
  const int operations[] = {WRITE_UPGRADE, DETACH_AND_BUS_RESET, REBOOT, REVERT_FACTORY, UPLOAD, UNKNOWN};
  for (int i = 0; operations[i] != UNKNOWN; i++) {
    if (strcmp(arg, operation_str(operations[i])) == 0)
      return operations[i];
  }
  return UNKNOWN;
}

struct options parse_arguments(int argc, char **argv)
{
  struct options o = {
    .operation = UNKNOWN,
    .arguments = {NULL, NULL},
    .device_id = {DFU_SUFFIX_IGNORE_ID, DFU_SUFFIX_IGNORE_ID,
                  DFU_SUFFIX_IGNORE_ID, I2C_ADDRESS_DEFAULT},
    .block_size = BLOCK_SIZE_DEFAULT
  };

  if (argc <= 1) {
    print_usage(stderr);
    exit(2);
  }

  int optind = 1;
  for (optind=1; optind<argc; optind++) {
    if ( (strcmp(argv[optind], "--help") == 0 ) || (strcmp(argv[optind], "-h") == 0) ) {
      print_usage(stderr);
      exit(2);
    } else if ( (strcmp(argv[optind], "--quiet") == 0 ) || (strcmp(argv[optind], "-q") == 0) ) {
      quiet = true;
      continue;
    } else if ( (strcmp(argv[optind], "--i2c-address") == 0 ) || (strcmp(argv[optind], "-i") == 0) ) {
      optind++;
      o.device_id.i2c_address = (uint8_t)strtol(argv[optind], NULL, 0);
      if (o.device_id.i2c_address == 0 && errno == EINVAL) {
        PRINT_ERROR("Invalid I2C address `%s'\n", argv[optind]);
      exit(1);
      }
      continue;
    } else if ( (strcmp(argv[optind], "--block-size") == 0 ) || (strcmp(argv[optind], "-b") == 0) ) {
      optind++;
      o.block_size = (unsigned)strtoul(argv[optind], NULL, 0);
      if (o.block_size == 0 && errno == EINVAL) {
        PRINT_ERROR("Invalid block size `%s'\n", argv[optind]);
        exit(1);
      }
      continue;
    } else {
      o.operation = parse_operation(argv[optind]);
      switch (o.operation) {
        case WRITE_UPGRADE:
          if (argc != optind + 2) {
            if (argc < optind + 2)
              PRINT_ERROR("Not enough command line arguments\n");
            else
              PRINT_ERROR("Too many command line arguments\n");

            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = argv[optind + 1];
          o.arguments[1] = NULL;
          break;

        case UPLOAD:
          if (argc != optind + 2) {
            if (argc < optind + 2)
              PRINT_ERROR("Not enough command line arguments\n");
            else
              PRINT_ERROR("Too many command line arguments\n");

            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = argv[optind + 1];
          o.arguments[1] = NULL;
          break;

        case DETACH_AND_BUS_RESET:
          if (argc != optind + 1) {
            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = NULL;
          o.arguments[1] = NULL;
          break;

        case REBOOT:
          if (argc != optind + 1) {
            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = NULL;
          o.arguments[1] = NULL;
          break;

        case REVERT_FACTORY:
          if (argc != optind + 1) {
            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = NULL;
          o.arguments[1] = NULL;
          break;

        default:
          PRINT_ERROR("Unknown operation \"%s\"\n", argv[optind]);
          print_usage(stderr);
          exit(1);
      }

      if (!quiet) {
        printf("options:\n");
        printf("- operation: ");
        switch (o.operation) {
          case WRITE_UPGRADE:
            printf("%s %s\n", operation_str(o.operation), o.arguments[0]);
            break;

          case DETACH_AND_BUS_RESET:
            printf("%s\n", operation_str(o.operation));
            break;

          case REBOOT:
            printf("%s\n", operation_str(o.operation));
            break;

          case REVERT_FACTORY:
            printf("%s\n", operation_str(o.operation));
            break;

          case UPLOAD:
            printf("%s\n", operation_str(o.operation));
            break;

          default:
            printf("%s\n", operation_str(o.operation));
            break;
        }
        printf("- I2C address 0x%02X\n", o.device_id.i2c_address);
        if (o.operation == WRITE_UPGRADE) {
          printf("- block size %u\n", o.block_size);
          printf("- block size %d\n", o.block_size);
        }
      }
      break;
    }
  }
  return o;
}
