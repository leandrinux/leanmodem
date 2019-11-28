#include "ansi.h"
#include "Stream.h"
#include "stdio.h"

Stream *ansi_stream;

void ansi_set_stream(Stream *stream) {
  ansi_stream = stream;
}

void ansi_clear() {
  ansi_stream->write("\e[2J");
}

void ansi_goto(int x, int y) {
  char str[15];
  sprintf(str, "\e[%d;%dH", y, x);
  ansi_stream->write(str);
}

void ansi_color(AnsiColor color) {
  char str[10];
  sprintf(str, "\e[%dm", color);
  ansi_stream->write(str);
}

void ansi_show_cursor(bool visible) {
  ansi_stream->write((visible) ? "\e[?25h" : "\e[?25l");
}

void ansi_move(AnsiDirection direction, int n) {
  char str[10];
  sprintf(str, "\e[%d%c", n, direction);
  ansi_stream->write(str);
}
