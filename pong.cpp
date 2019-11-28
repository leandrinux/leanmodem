#include "pong.h"
#include "Arduino.h"
#include "ansi.h"
#include "config.h"

#define CHR_HORIZONTAL_LINE '═'
#define CHR_VERTICAL_SEPARATOR '|'
#define CHR_BALL 'O'
#define FONT_CHAR_WIDTH 9
#define FONT_CHAR_HEIGHT 4

typedef char Character[FONT_CHAR_HEIGHT][FONT_CHAR_WIDTH];
typedef enum Alignment { left_side, right_side };
typedef enum Paddle { left_paddle, right_paddle };

const char title_1[] = "|  _ \\ / _ \\| \\ | |/ ___|";
const char title_2[] = "| |_) | | | |  \\| | |  _ ";
const char title_3[] = "|  __/| |_| | |\\  | |_| |";
const char title_4[] = "|_|    \\___/|_| \\_|\\____|";

Character font_char0 {
  " / _ \\ ", "| | | |", "| |_| |", " \\___/ "
};

Character font_char1 {
  "/ |", "| |", "| |", "|_|"
};

Character font_char2 {
  "|___ \\ ", "  __) |", " / __/ ", "|_____|"
};

Character font_char3 {
  "|___ / ", "  |_ \\ ", " ___) |", "|____/ "
};

Character font_char4 {
  "| || |  ", "| || |_ ", "|__   _|", "   |_|  "
};

Character font_char5 {
  "| ___| ", "|___ \\ ", " ___) |", "|____/ "
};

Character font_char6 {
   " / /_  ", "| '_ \\ ", "| (_) |", " \\___/ "
};

Character font_char7 {
  "|___  |", "   / / ", "  / /  ", " /_/   "
};

Character font_char8 {
  " ( _ ) ", " / _ \\ ", "| (_) |", " \\___/ "
};

Character font_char9 {
  " / _ \\ ", "| (_) |", " \\__, |", "   /_/ "
};

Character *font[] = { 
  &font_char0, &font_char1, &font_char2, &font_char3, &font_char4, 
  &font_char5, &font_char6, &font_char7, &font_char8, &font_char9 
};

const char subtitle[] = "press q to quit. move using d & c for left, k & m for right";
const char left_wins[] = "- LEFT WINS -";
const char right_wins[] = "- RIGHT WINS -";
const char blank[] = "              ";  
const byte screen_width = 80;
const byte screen_height = 24;
const byte play_area_top = 5;
const byte play_area_bottom = screen_height - 2;
const byte play_area_left = 8;
const byte play_area_right = screen_width - 8;
const byte middle = screen_width / 2;
const AnsiColor bg_color = bgBlue;
const AnsiColor fg_color = fgYellow;
const AnsiColor score_color = fgGreen;
const char paddle_symbol = 0xB0;
const byte paddle_height = 4;
const byte paddle_margin = 4;
const byte initial_ball_x = screen_width / 2;
const byte initial_ball_y = 1 + play_area_top + (play_area_bottom - play_area_top) / 2; 
const byte initial_left_paddle_y = (play_area_bottom + play_area_top) / 2 - paddle_height + 3;
const byte initial_right_paddle_y = initial_left_paddle_y;
const byte frame_delay = 60;

byte game_end = false;
byte left_score = 0;
byte right_score = 0;
byte left_paddle_y = initial_left_paddle_y;
byte right_paddle_y = initial_right_paddle_y;
byte ball_x = initial_ball_x;
byte ball_y = initial_ball_y; 
int ball_inc_x = 1;
int ball_inc_y = 1;
Stream *pong_stream = NULL;

void pong_end();

void snd_score() {
  tone(CFG_BUZZER_PIN, 1800, 100);
}

void snd_wall_bounce() {
  tone(CFG_BUZZER_PIN, 1000, 100);
}

void snd_paddle_bounce() {
  tone(CFG_BUZZER_PIN, 2000, 50);
}

void snd_gameover() {
  tone(CFG_BUZZER_PIN, 1000, 100); delay(100);
  tone(CFG_BUZZER_PIN, 1000, 100); delay(100);
  tone(CFG_BUZZER_PIN, 1000, 300); delay(300); 
}

void draw_hline(int x1, int x2, int y, char c) {
  for(int i = x1 ; i <= x2 ; i++) {
    ansi_goto(i, y);
    pong_stream->write(c);
  }
}

void draw_vline(int x, int y1, int y2, char c) {
  for(int i = y1 ; i <= y2 ; i++) {
    ansi_goto(x, i);
    pong_stream->write(c);
  }  
}

void draw_paddle(Paddle paddle, char symbol) {
  int x, y1, y2;
  if (paddle == left_paddle) {
    y1 = left_paddle_y;
    x = play_area_left + paddle_margin;  
  } else {
    y1 = right_paddle_y;
    x = play_area_right - paddle_margin;      
  }
  y2 = y1 + paddle_height;  
  for (int i=y1; i<y2; i++) {
    ansi_goto(x, i);
    pong_stream->print(symbol);
  }
}

void write_digit(int digit, int x, int y, Alignment alignment) {
  Character *pChar = font[digit];
  char empty[FONT_CHAR_WIDTH];
  char *str;
  memset(empty, ' ', sizeof(empty));
  for (int i=0 ; i<FONT_CHAR_HEIGHT ; i++) {
    ansi_goto(x, y+i);  
    str = (*pChar)[i]; 
    if (alignment == left_side) {
      pong_stream->write(str);
      pong_stream->write((byte *)&empty, FONT_CHAR_WIDTH-strlen(str));      
    } else {
      pong_stream->write((byte *)&empty, FONT_CHAR_WIDTH-strlen(str));            
      pong_stream->write(str);
    }
  }
}

void move_paddle(Paddle paddle, int inc) {
  byte x;
  byte *y;
  if (paddle == left_paddle) {
    x =  play_area_left + paddle_margin;
    y = &left_paddle_y;  
  } else {
    x =  play_area_right - paddle_margin;
    y = &right_paddle_y;      
  }
  if ( (inc<0) && (*y == play_area_top + 1)) return;
  if ( (inc>0) && (*y == play_area_bottom - paddle_height)) return;
  if (inc>0) {
    ansi_goto(x, *y); pong_stream->print(' ');
    ansi_goto(x, *y+paddle_height); pong_stream->print(paddle_symbol);
  } else {
    ansi_goto(x, *y-1); pong_stream->print(paddle_symbol);
    ansi_goto(x, *y+paddle_height-1); pong_stream->print(' ');    
  }
  *y += inc;
}

void draw_score(int score, int x, Alignment alignment) {
  ansi_color(score_color);
  const int y = 1;
  int lo = score % 10;
  ansi_goto(1, 10);  
  if (alignment == left_side) {
    write_digit(lo, x, y, alignment);
  } else {
    write_digit(lo, x - FONT_CHAR_WIDTH, y, alignment);
  }
  ansi_color(fg_color);
}

void reset_ball() {
  ansi_goto(ball_x, ball_y);
  pong_stream->write(CHR_BALL);  
}

bool move_ball() {
  ansi_goto(ball_x, ball_y);
  pong_stream->write((ball_x == middle) ? CHR_VERTICAL_SEPARATOR : ' ');
  ball_x += ball_inc_x;
  ball_y += ball_inc_y;
  byte ball_ahead_x = ball_x + ball_inc_x;
  byte ball_ahead_y = ball_y + ball_inc_y;
  ansi_goto(ball_x, ball_y);
  pong_stream->write(CHR_BALL);

  // ball hits left side?
  if (ball_x == play_area_left) {
    snd_score(); 
    ball_inc_x = -ball_inc_x;
    right_score += 1;
    draw_score(right_score, play_area_right + 1, right_side);
    return true;
  }

  // ball hits right side?
  if (ball_x == play_area_right) {
    snd_score(); 
    ball_inc_x = -ball_inc_x;
    left_score += 1;
    draw_score(left_score, play_area_left, left_side);
    return true;
  }  

  // ball hits lower or upper walls?
  if ((ball_y == play_area_top + 1) || (ball_y == play_area_bottom - 1)) { 
    snd_wall_bounce(); 
    ball_inc_y = -ball_inc_y; 
  }

  // ball hits left paddle?
  if ( (ball_ahead_x == play_area_left + paddle_margin) &&
       (ball_ahead_y >= left_paddle_y) &&
       (ball_ahead_y <= left_paddle_y + paddle_height) ) {
    snd_paddle_bounce(); 
    ball_inc_x = -ball_inc_x;    
  }

  // ball hits right paddle?
  if ( (ball_ahead_x == play_area_right - paddle_margin) &&
       (ball_ahead_y >= right_paddle_y) &&
       (ball_ahead_y <= right_paddle_y + paddle_height) ) {
    snd_paddle_bounce(); 
    ball_inc_x = -ball_inc_x;    
  }

  return false;
}

bool listen_keyboard() {
  if (pong_stream->available()) {
    int key = pong_stream->read();
    if (key>0) {
      switch((char)key) {
        case 'q':
          pong_end();
          return true;
          break;
        case 'd':
          move_paddle(left_paddle, -1);
          break;
        case 'k':
          move_paddle(right_paddle, -1);
          break;
        case 'c':
          move_paddle(left_paddle, 1);
          break;
        case 'm':
          move_paddle(right_paddle, 1);
          break;
      }
    }
  }  
  return false;
}

void reset_items() {

  // reset the ball
  ansi_goto(ball_x, ball_y);
  pong_stream->write(' ');
  ball_x = initial_ball_x;
  ball_y = initial_ball_y;
  ansi_goto(ball_x, ball_y);
  pong_stream->write(CHR_BALL);

  // reset the paddles
  draw_paddle(left_paddle, ' ');
  draw_paddle(right_paddle, ' ');
  left_paddle_y = initial_left_paddle_y;
  right_paddle_y = initial_right_paddle_y;
  draw_paddle(left_paddle, paddle_symbol);
  draw_paddle(right_paddle, paddle_symbol);
}

void game_loop() {
  bool game_over = false;
  bool score = false;
  
  left_score = 0;
  right_score = 0;
  draw_score(left_score, play_area_left, left_side);
  draw_score(right_score, play_area_right, right_side);

  reset_items();
  delay(2000);
  while(!game_over && !game_end) {
    game_end = listen_keyboard();
    score = move_ball();
    if (score) {
      game_over = (left_score>9) || (right_score>9);
      if (!game_over) {
        delay(3000);
        reset_items();        
      }
    } else {
      delay(frame_delay);      
    }
  }

  if (game_end) return;
  snd_gameover();
  char *win_text = (left_score > right_score) ? (char *)&left_wins : (char *)&right_wins; 
  ansi_goto((screen_width / 2)-((strlen(win_text)-1)/2), play_area_bottom + 1);
  pong_stream->print(win_text);
  delay(5000);
  ansi_goto((screen_width / 2) - 7, play_area_bottom + 1);
  pong_stream->print(blank);
}

void draw_field() { 

  // Pong title
  const int title_x = (screen_width/2)-(sizeof(title_1)/2);
  const int title_y = 1;
  ansi_goto(title_x, title_y); pong_stream->write(title_1, sizeof(title_1)-1);
  ansi_goto(title_x, title_y+1); pong_stream->write(title_2, sizeof(title_2)-1);
  ansi_goto(title_x, title_y+2); pong_stream->write(title_3, sizeof(title_3)-1);
  ansi_goto(title_x, title_y+3); pong_stream->write(title_4, sizeof(title_4)-1);

  // Text below the game field
  ansi_goto((screen_width / 2)-((sizeof(subtitle)-1)/2), play_area_bottom + 2);
  pong_stream->write(subtitle, sizeof(subtitle)-1);

  // Field lines
  draw_hline(play_area_left, play_area_right, play_area_top, CHR_HORIZONTAL_LINE);
  draw_hline(play_area_left, play_area_right, play_area_bottom, CHR_HORIZONTAL_LINE);
  draw_vline(play_area_left + (play_area_right-play_area_left) / 2, play_area_top, play_area_bottom, CHR_VERTICAL_SEPARATOR);
}

void pong_start(Stream *stream) {
  pong_stream = stream;
  ansi_show_cursor(false);
  ansi_color(bg_color);
  ansi_color(fg_color);
  ansi_clear();
  
  draw_field();
  while (!game_end) game_loop();
  pong_end();
}

void pong_end() {
  ansi_color(none);
  ansi_clear();
  ansi_goto(1, 1);
  ansi_show_cursor(true);
}
