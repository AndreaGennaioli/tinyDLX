#include "dlx_terminal.h"
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios original_termios;

static void restore_terminal(int sig) {
  tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
  exit(sig);
}

void dlx_terminal_raw() {
  tcgetattr(STDIN_FILENO, &original_termios);
  signal(SIGINT, restore_terminal);
  signal(SIGTERM, restore_terminal);

  struct termios t = original_termios;
  t.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void dlx_terminal_restore() {
  tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}
