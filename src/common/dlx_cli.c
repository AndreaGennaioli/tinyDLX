#include "dlx_cli.h"
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_help(const char *program_name, FILE *output);
static int check_config(const char *program_name, DLX_config *config);

static int parse_u64(const char *str, uint64_t *out);
static int parse_u32(const char *str, uint32_t *out);

int parse_arguments(int argc, char **argv, DLX_config *config) {
  int opt;
  const struct option longopts[] = {
      {"binary-file", required_argument, NULL, 'b'},
      {"help", no_argument, NULL, 'h'},
      {"strict", no_argument, NULL, 's'},
      {"freq", required_argument, NULL, 'f'},
      {"max-cycles", required_argument, NULL, 'C'},
      {"init-gpr", required_argument, NULL, 'G'},
      {"dump-state", required_argument, NULL, 'd'},
      {0, 0, 0, 0},
  };

  while ((opt = getopt_long(argc, argv, "b:f:C:G:d:sh", longopts, NULL)) != -1) {
    switch (opt) {
    case 'b':
      config->program_file = optarg;
      break;
    case 'f':
      if(parse_u32(optarg, &config->freq_hz) == 0) {
        fprintf(stderr, "An error while parsing --freq value.");
        exit(EXIT_FAILURE);
      }
      break;
    case 'C':
      if(parse_u64(optarg, &config->max_cycles) == 0) {
        fprintf(stderr, "An error while parsing --max-cycles value.");
        exit(EXIT_FAILURE);
      }
      break;
    case 'G':
      if(parse_u32(optarg, &config->init_gpr) == 0) {
        fprintf(stderr, "An error while parsing --init-gpr value.");
        exit(EXIT_FAILURE);
      }
      config->init_gpr_set = 1;
      break;
    case 'd':
      config->dump_state = optarg;
      break;
    case 's':
      config->strict_mode = 1;
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

static int parse_u64(const char *str, uint64_t *out) {
  const char *p = str;
  while (isspace((unsigned char)*p)) p++;
  if(*p == '-' || *p == '+') return 0;

  // strtoull does not set errno to 0 if success
  errno = 0;
  char *end;
  unsigned long long v = strtoull(p, &end, 0);
  if(end == p) return 0;
  if(*end != '\0') return 0;
  if(errno == ERANGE) return 0;

  *out = (uint64_t)v;
  return 1;
}

static int parse_u32(const char *str, uint32_t *out) {
  uint64_t v;
  if(parse_u64(str, &v) == 0) return 0;
  if(v > UINT32_MAX) return 0;
  *out = (uint32_t)v;
  return 1;
}

static int check_config(const char *program_name, DLX_config *config) {
  if (config->program_file == NULL || strlen(config->program_file) == 0) {
    fprintf(stderr, "An input binary file is needed.\n");
    print_help(program_name, stderr);
    return 0;
  }

  if (config->dump_state){
    if(strlen(config->dump_state) == 0) {
      fprintf(stderr, "Please specify a valid dump filename.\n");
      print_help(program_name, stderr);
      return 0;
    }

    FILE *f = fopen(config->dump_state, "a");
    if(f == NULL){
      fprintf(stderr, "Cannot open dump file '%s': %s", config->dump_state, strerror(errno));
      return 0;
    }
    fclose(f);
  }

  return 1;
}

static void print_help(const char *program_name, FILE *output) {
  fprintf(output, "Usage: %s -b <program.bin> [options]\n\n", program_name);
  fprintf(output, "tinyDLX - DLX emulator\n\n");
  fprintf(output, "Options:\n");
  fprintf(output, "  -b, --binary-file FILE    Binary program file"
                  "(required)\n");
  fprintf(output, "  -f, --freq FREQUENCY      Target frequency of execution, if not specified or 0, full use of host CPU (core) is expected\n");
  fprintf(output, "  -C, --max-cycles CYCLES   Maximum number of execution cycles\n");
  fprintf(output, "  -G, --init-gpr VALUE      Init value for GPRs, only absolute values. (by default GPRs values are non-deterministic!)\n");
  fprintf(output, "  -d, --dump-state PATH     Writes a JSON snapshot of the state of DLX at the end of execution\n");
  fprintf(output, "  -s, --strict              Run emulation in strict mode: warns are now faults\n");
  fprintf(output,
          "  -h, --help                Shows this help comand and exits\n");
}
