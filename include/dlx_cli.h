#ifndef DLX_CLI_H
#define DLX_CLI_H

#include "dlx_defs.h"

// Parses command line arguments and populates DLX_config
int parse_arguments(int argc, char **argv, DLX_config *config);

#endif // !DLX_CLI_H
