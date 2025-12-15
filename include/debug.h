#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

// ANSI terminal colors
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"

#define error(fmt, ...) fprintf(stderr, COLOR_RED "[ERROR] " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define warn(fmt, ...) fprintf(stderr, COLOR_YELLOW "[WARN] " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define info(fmt, ...) fprintf(stderr, "[INFO] " fmt "\n", ##__VA_ARGS__)

#endif // !DEBUG_H
