#include "dlx_cli.h"
#include "getopt.h"
#include <bits/getopt_ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_help(const char *program_name, FILE *output);
static int check_config(const char *program_name, DLX_config *config);

int parse_arguments(int argc, char **argv, DLX_config *config) {
  int opt;
  const struct option longopts[] = {
      {"binary-file", required_argument, NULL, 'b'},
      {"help", no_argument, NULL, 'h'},
      {0, 0, 0, 0},
  };

  while ((opt = getopt_long(argc, argv, "b:h", longopts, NULL)) != -1) {
    switch (opt) {
    case 'b':
      config->program_file = optarg;
      break;
    case 'h':
      print_help(argv[0], stdout);
      exit(EXIT_SUCCESS);
    case '?':
    default:
      print_help(argv[0], stderr);
      exit(EXIT_FAILURE);
    }
  }

  return check_config(argv[0], config);
}

static int check_config(const char *program_name, DLX_config *config) {
  if (strlen(config->program_file) == 0) {
    fprintf(stderr, "An input binary file is needed.\n");
    print_help(program_name, stderr);
    return 0;
  }

  return 1;
}

static void print_help(const char *program_name, FILE *output) {
  fprintf(output, "Usage: %s -b <program.bin> [options]\n\n", program_name);
  fprintf(output, "tinyDLX - DLX emulator\n\n");
  fprintf(output, "Options:\n");
  fprintf(output, "  -b, --binary-file FILE    Binary program file"
                  "(required)\n");
  fprintf(output,
          "  -h, --help                Shows this help comand and exits\n");
}
