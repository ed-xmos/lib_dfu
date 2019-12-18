// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/errno.h>
#include "options.h"

static struct option option_spec[] = {
  { "regular-sector-size", required_argument, NULL, 's' },
  { "verbose", no_argument, NULL, 'v' },
  { "factory", required_argument, NULL, 'f' },
  { "upgrade", required_argument, NULL, 'u' },
  { "output", required_argument, NULL, 'o' },
  { NULL, 0, NULL, 0 }
};

static const char usage[] = "\
usage: data_partition_generator [--verbose] [FACTORY] [UPGRADE] -o OUTFILE\n\
       FACTORY =   --factory FACTORY_FILE\n\
       UPGRADE =   --upgrade COMPATIBILITY_VERSION UPGRADE_FILE\n\
\n\
       COMPATIBILITY_VERSION is redundant and provided for consistency with xflash\
";

struct options parse_command_line(int argc, char **argv)
{
  struct options o = {
    .regular_sector_size = -1,
    .verbose = false,
    .factory_file_name = NULL,
    .upgrade_comp_version = -1,
    .upgrade_file_name = NULL,
    .out_file_name = NULL
  };

  int ch;

  do {
    ch = getopt_long(argc, argv, "s:vf:u:o:", option_spec, NULL);
    switch (ch) {
      case 's':
        o.regular_sector_size = strtol(optarg, NULL, 10);
        if (o.regular_sector_size == 0 && errno == EINVAL) {
          fprintf(stderr, "error: invalid upgrade compatibility version `%s'\n", optarg);
          exit(1);
        }
        break;

      case 'v':
        o.verbose = true;
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

  if (o.verbose) {
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
