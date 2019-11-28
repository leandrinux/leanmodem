#ifndef ansi_h
#define ansi_h

#include "Stream.h"

typedef enum {
  fgBlack = 30,
  fgRed = 31,
  fgGreen = 32,
  fgYellow = 33,
  fgBlue = 34,
  fgMagenta = 35,
  fgCyan = 36,
  fgWhite = 37,
  bgBlack = 40,
  bgRed = 41,
  bgGreen = 42,
  bgYellow = 43,
  bgBlue = 44,
  bgMagenta = 45,
  bgCyan = 46,
  bgWhite = 47,
  none = 0,
  bold = 1  
} AnsiColor;

typedef enum {
  up = 'A',
  down = 'B',
  right = 'C',
  left = 'D'
} AnsiDirection;

void ansi_set_stream(Stream *stream);
void ansi_clear();
void ansi_goto(int x, int y);
void ansi_color(AnsiColor color);
void ansi_show_cursor(bool visible);
void ansi_move(AnsiDirection direction, int n);

#endif
