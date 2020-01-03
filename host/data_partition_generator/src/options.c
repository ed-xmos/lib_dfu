// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/errno.h>
#include "options.h"

bool verbose = false;

static struct option option_spec[] = {
  { "help", no_argument, NULL, 'h' },
  { "help-advanced", no_argument, NULL, 'a' },
  { "regular-sector-size", required_argument, NULL, 's' },
  { "verbose", no_argument, NULL, 'v' },
  { "factory", required_argument, NULL, 'f' },
  { "upgrade", required_argument, NULL, 'u' },
  { "output", required_argument, NULL, 'o' },
  { "bad-factory-crc", no_argument, NULL, 'c' },
  { "bad-upgrade-crc", no_argument, NULL, 'd' },
  { NULL, 0, NULL, 0 }
};

static const char usage[] = "\
usage: data_partition_generator --help\n\
\n\
       data_partition_generator [--verbose] [FACTORY] [UPGRADE] -o OUTFILE\n\
\n\
       FACTORY =   --factory FACTORY_FILE\n\
       UPGRADE =   --upgrade COMPATIBILITY_VERSION UPGRADE_FILE\n\
\n\
       COMPATIBILITY_VERSION is redundant and provided for consistency with xflash\n\
";

static const char advanced_usage[] = "\
\n\
       --bad-factory-crc    invert CRC for test purposes\n\
       --bad-upgrade-crc    invert CRC for test purposes\n\
";

struct options parse_command_line(int argc, char **argv)
{
  struct options o = {
    .regular_sector_size = -1,
    .factory_file_name = NULL,
    .upgrade_comp_version = -1,
    .upgrade_file_name = NULL,
    .out_file_name = NULL,
    .bad_factory_crc = false,
    .bad_upgrade_crc = false
  };

  int ch;

  do {
    ch = getopt_long(argc, argv, "has:vf:u:o:c", option_spec, NULL);
    switch (ch) {
      case 'h':
        fprintf(stderr, usage);
        exit(2);
        break;

      case 'a':
        fprintf(stderr, usage);
        fprintf(stderr, advanced_usage);
        exit(2);
        break;

      case 's':
        o.regular_sector_size = strtol(optarg, NULL, 10);
        if (o.regular_sector_size == 0 && errno == EINVAL) {
          fprintf(stderr, "error: invalid upgrade compatibility version `%s'\n", optarg);
          exit(1);
        }
        break;

      case 'v':
        verbose = true;
        break;

      case 'f':
        o.factory_file_name = optarg;
        break;

      case 'u':
        o.upgrade_comp_version = strtol(optarg, NULL, 10);
        if (o.upgrade_comp_version == 0 && errno == EINVAL) {
          fprintf(stderr, "error: invalid upgrade compatibility version `%s'\n", optarg);
          exit(1);
        }
        optarg = argv[optind];
        optind++;
        o.upgrade_file_name = optarg;
        break;

      case 'o':
        o.out_file_name = optarg;
        break;

      case 'c':
        o.bad_factory_crc = true;
        break;

      case 'd':
        o.bad_upgrade_crc = true;
        break;

      case -1:
        break;

      default:
        fprintf(stderr, usage);
        exit(1);
    }
  } while (ch != -1);

  if (o.factory_file_name == NULL && o.upgrade_file_name == NULL) {
    fprintf(stderr, "error: neither factory nor upgrade specified\n");
    fprintf(stderr, usage);
    exit(1);
  }
  if (o.regular_sector_size == -1) {
    fprintf(stderr, "error: sector size not specified\n");
    exit(1);
  }
  if (o.upgrade_file_name != NULL && o.upgrade_comp_version == -1) {
    fprintf(stderr, "error: upgrade file specified but not compatibility version\n");
    exit(1);
  }
  if (o.upgrade_file_name == NULL && o.upgrade_comp_version != -1) {
    fprintf(stderr, "error: upgrade compatibility version specified but not file\n");
    exit(1);
  }

  if (optind < argc) {
    fprintf(stderr, "error: unexpected command line arguments:");
    for (int i = optind; i < argc; i++) {
      fprintf(stderr, " %s", argv[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, usage);
    exit(1);
  }

  if (verbose) {
    printf("options: ");
    printf("factory file name %s, ",
           o.factory_file_name == NULL ? "-" : o.factory_file_name);
    printf("upgrade compatibility version %d, ",
           o.upgrade_comp_version);
    printf("upgrade file name %s, ",
           o.upgrade_file_name == NULL ? "-" : o.upgrade_file_name);
    printf("out file name %s\n",
           o.out_file_name == NULL ? "-" : o.out_file_name);
  }

  return o;
}
