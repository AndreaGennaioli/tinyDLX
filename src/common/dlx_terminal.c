#include "dlx_terminal.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios original_termios;

void dlx_terminal_raw() {
  tcgetattr(STDIN_FILENO, &original_termios);
  atexit(dlx_terminal_restore);

  struct termios t = original_termios;
  t.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void dlx_terminal_restore() {
  tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}
