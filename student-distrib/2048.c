// #include <boot/stivale2.h>
// #include <devices/keyboard/keyboard.h>
#include "pit.h"
#include "RTC.h"
#include "2048.h"
#include "keyboard.h"
// #include "draw.h"
// #include "screen.h"
// #include <devices/serial/serial.h>
// #include "libasm/asm.h"
// #include "liblog/logging.h"
// #include "libmath/math.h"
// #include "librand/random.h"
// #include "libstr/string.h"
// #include <stddef.h>
// #include <stdint.h>
// #include <system/gdt/gdt.h>
// #include <system/interrupts/idt.h>

#define bool uint8_t
#define true 1
#define false 0
#define TIMER_HZ 363
#define FPS 30
// uint32_t buffers[2][BUFFER_SIZE];
// uint8_t bck = 0;

// #define CURRENT (buffers[bck])
// #define SWAP() (bck = 1 - bck)

// Colours
// colour_t tile_colours[] = {
//     RGB(205, 193, 180),  // 0
//     RGB(238, 228, 218),  // 2
//     RGB(237, 224, 200),  // 4
//     RGB(242, 177, 121),  // 8
//     RGB(245, 149, 99),   // 16
//     RGB(246, 124, 95),   // 32
//     RGB(246, 94, 59),    // 64
//     RGB(237, 207, 114),  // 128
//     RGB(237, 204, 97),   // 256
//     RGB(237, 200, 80),   // 512
//     RGB(237, 197, 63),   // 1024
//     RGB(237, 194, 46)    // 2048
// };

// colour_t tile_text_colours[] = {
//     RGB(0, 0, 0),        // 0; no text for empty
//     RGB(119, 110, 101),  // 2
//     RGB(119, 110, 101),  // 4
//     RGB(249, 246, 242),  // 8
//     RGB(249, 246, 242),  // 16
//     RGB(249, 246, 242),  // 32
//     RGB(249, 246, 242),  // 64
//     RGB(249, 246, 242),  // 128
//     RGB(249, 246, 242),  // 256
//     RGB(249, 246, 242),  // 512
//     RGB(249, 246, 242),  // 1024
//     RGB(249, 246, 242),  // 2048
// };

#define BLACK RGB(0, 0, 0)
#define WHITE RGB(255, 255, 255)
#define BROWN RGB(120, 110, 101)

#define GRID_COLOUR RGB(187, 173, 160)

// Positioning
#define BOX_SIZE 25
#define PADDING 5

#define CENTER_X (SCREEN_WIDTH / 2)
#define CENTER_Y (SCREEN_HEIGHT / 2)

#define CENTER_TITLE_Y (CENTER_Y / 4)
#define CENTER_GRID_Y (5 * CENTER_Y / 4)

#define GRID_SIZE (5 * PADDING + 4 * BOX_SIZE)
#define GRID_X CENTER_X - (GRID_SIZE / 2)
#define GRID_Y CENTER_GRID_Y - (GRID_SIZE / 2)

#define DIALOG_PADDING 10
#define DIALOG_WIDTH 75
#define DIALOG_HEIGHT 50
#define DIALOG_X CENTER_X - (DIALOG_WIDTH / 2)
#define DIALOG_Y CENTER_GRID_Y - (DIALOG_HEIGHT / 2)

#define DIALOG_PADDING_WIDTH (DIALOG_WIDTH + DIALOG_PADDING)
#define DIALOG_PADDING_HEIGHT (DIALOG_HEIGHT + DIALOG_PADDING)
#define DIALOG_PADDING_X CENTER_X - (DIALOG_PADDING_WIDTH / 2)
#define DIALOG_PADDING_Y CENTER_GRID_Y - (DIALOG_PADDING_HEIGHT / 2)

// State
#define NUM_CONTROLS 5

uint32_t pow(int inp,int vals){
  int i=0,exp=1;
  for (i=0;i<vals;i++)
    exp=exp*inp;
  return exp;
}

static uint32_t next = 1;

int rand(void) {
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }

struct psf fb_font;

struct control {
  bool down;
  bool last;
  bool pressed;
};

static struct {
  uint16_t grid[4][4];
  uint16_t score;
  enum { NOT_OVER, WIN, LOSS } progress;
  union {
    struct {
      struct control up;
      struct control left;
      struct control down;
      struct control right;
      struct control restart;
    };
    struct control raw[NUM_CONTROLS];
  } controls;
} state;

void compress() {
  int x,y;
  for (x = 0; x < 4; x++) {
    int pos = 0;
    for (y = 0; y < 4; y++) {
      if (state.grid[x][y] != 0) {
        state.grid[x][pos] = state.grid[x][y];
        if (y != pos) {
          state.grid[x][y] = 0;
        }
        pos += 1;
      }
    }
  }
}

void merge() {
  int x,y;
  for (x = 0; x < 4; x++) {
    for (y = 0; y < 3; y++) {
      if (state.grid[x][y] == state.grid[x][y + 1] && state.grid[x][y] != 0) {
        state.grid[x][y] = state.grid[x][y] + 1;
        state.grid[x][y + 1] = 0;

        state.score += pow(2, state.grid[x][y]);
      }
    }
  }
}

void reverse() {
  int x,y;
  for (x = 0; x < 4; x++) {
    for (y = 0; y < 2; y++) {
      uint16_t temp = state.grid[x][y];
      state.grid[x][y] = state.grid[x][3 - y];
      state.grid[x][3 - y] = temp;
    }
  }
}

void transpose() {
  int x,y;
  for (x = 0; x < 4; x++) {
    for (y = x + 1; y < 4; y++) {
      uint16_t temp = state.grid[x][y];
      state.grid[x][y] = state.grid[y][x];
      state.grid[y][x] = temp;
    }
  }
}

void move_up() {
  compress();
  merge();
  compress();
}

void move_left() {
  transpose();
  move_up();
  transpose();
}

void move_down() {
  reverse();
  move_up();
  reverse();
}

void move_right() {
  transpose();
  move_down();
  transpose();
}

void insert_new() {
  int x = rand() % 4;
  int y = rand() % 4;

  int i = 0;

  while (state.grid[x][y] != 0) {
    // avoids getting stuck in while loop
    if (i > 16) {
      return;
    }

    x = rand() % 4;
    y = rand() % 4;
    i++;
  }

  // 10% chance of a 4, 90% chance of 2
  int chance = rand() % 10;
  if (chance == 0) {
    state.grid[x][y] = 2;
  } else {
    state.grid[x][y] = 1;
    printf("X=%d y=%d value=%d",x,y,state.grid[x][y]);
  }
}

void update_game_progress() {
  int x, y;
  int hasEmptyCell = 0;

  // Check for winning condition and empty cells
  for (x = 0; x < 4; x++) {
    for (y = 0; y < 4; y++) {
      if (state.grid[x][y] == 11) {
        state.progress = WIN;
        return;
      }
      if (state.grid[x][y] == 0) {
        hasEmptyCell = 1;
      }
      if (x < 3 && state.grid[x][y] == state.grid[x + 1][y]) {
        state.progress = NOT_OVER;
        return;
      }
      if (y < 3 && state.grid[x][y] == state.grid[x][y + 1]) {
        state.progress = NOT_OVER;
        return;
      }
    }
  }

  if (hasEmptyCell) {
    // There is an empty cell, game is not over
    state.progress = NOT_OVER;
  } else {
    state.progress = LOSS;
  }
}

void render_grid_background(void) {
  // draw_rect(GRID_X, GRID_Y, GRID_SIZE, GRID_SIZE, GRID_COLOUR);
  int x=0,y=0;
  for (x=0;x<20;x++){
    for(y=0+12;y<10+12;y=y+2){
      update_char('_',y,x+20);
    }
  }
  for (x=0;x<21;x=x+5){
    for(y=13;y<9+12;y++){
      update_char('|',y,x+20);
    }
  }
}

void render_ui(void) {
  // font_puts_center("2048os", CENTER_X, CENTER_TITLE_Y, 10, BROWN, BLACK);
  // font_puts_center("gitlab.com/Jpac14/2048-os", CENTER_X,
  //                  CENTER_TITLE_Y + FONT_HEIGHT * 6, 2, WHITE, BLACK);

  // font_puts("Score:", GRID_X, GRID_Y - FONT_HEIGHT * 3, 2, WHITE, BLACK);
  // font_puts("wasd or arrows to move grid; r to restart", GRID_X,
  //           GRID_Y + GRID_SIZE + FONT_HEIGHT, 2, WHITE, BLACK);

}

void render_boxes(void) {
  int x, y,z;
  int a=0,b=0;
  for (x = 0; x < 4; x++) {
    for (y = 0; y <4; y++) {
      char buffer [1];
      if (state.grid[a][b] != 0){
      itoa(pow(2,state.grid[a][b]),buffer,10);
      z=0;
      while (buffer[z]!='\0'){
        if ((read_char(y,x)!='|')&&(read_char(y,x)!='_')){
        update_char(buffer[z],y*2+z+21,x*5+z+1);}
        z++;
      }

      }
      else{
        for (z=0;z<4;z++){
          update_char('0',y*2+z+21,x*5+z+1);
        }
      }

    }

  }
}

void render_score(void) {
  char* buf=itoa(state.score,buf, 10);
  // font_puts(buf, GRID_X + FONT_WIDTH * 14,
  //           GRID_Y - FONT_HEIGHT * 3, 2, WHITE, BLACK);
  int x=0,y=0;
  for (x=state.score;x>1;x=x/10){
      update_char(buf[y],2,x);
      y++;
  }

}

void render_dialog(void) {
  draw_rect(DIALOG_PADDING_X, DIALOG_PADDING_Y, DIALOG_PADDING_WIDTH,
            DIALOG_PADDING_HEIGHT, WHITE);
  draw_rect(DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT, BROWN);
}

void render_win(void) {
  render_dialog();

  font_puts_center("You Win!", CENTER_X, CENTER_GRID_Y - FONT_HEIGHT, 3,
                   WHITE, BROWN);
  font_puts_center("press r to restart", CENTER_X,
                   CENTER_GRID_Y + (FONT_HEIGHT * 2), 2, WHITE, BROWN);
}

void render_loss(void) {
  render_dialog();

  font_puts_center("You Lose!", CENTER_X, CENTER_GRID_Y - FONT_HEIGHT, 2,
                   WHITE, BROWN);
  font_puts_center("press r to restart", CENTER_X,
                   CENTER_GRID_Y + (FONT_HEIGHT * 2), 1, WHITE, BROWN);
}

void render(void) {
  // clear_screen();
  // set_screen(get_colour(&BLACK));
  
  // render_ui();
  render_boxes();
  // render_score();

  // switch (state.progress) {
  //   case WIN:
  //     render_win();
  //     break;
  //   case LOSS:
  //     render_loss();
  //     break;
  //   case NOT_OVER:
  //     break;
  // }
}
// void set_screen(uint32_t colour) { memset(&CURRENT, colour, BUFFER_SIZE); }
void start_game(void) {
  int x, y;
  for (x = 0; x < 4; x++) {
    for (y = 0; y < 4; y++) {
      state.grid[x][y] = 0;
    }
  }

  state.progress = NOT_OVER;
  state.score = 0;
  clear_screen();
  render_grid_background();
  insert_new();
  insert_new();
  // render_boxes();
}

void update(void) {
  // Update control states
  // clang-format off
  bool control_states[] = {
    keyboard_char('w') || keyboard_key(KEY_UP),
    keyboard_char('a') || keyboard_key(KEY_LEFT),
    keyboard_char('s') || keyboard_key(KEY_DOWN),
    keyboard_char('d') || keyboard_key(KEY_RIGHT),
    keyboard_char('r')
  };
  // clang-format on
  int i=0;
  for (i = 0; i < NUM_CONTROLS; i++) {
    struct control *c = &state.controls.raw[i];
    c->last = c->down;
    c->down = control_states[i];
    c->pressed = !c->last && c->down;
  };

  bool inputDetected = false;
  if (state.controls.up.pressed) {
    move_up();
    printf("up");
    inputDetected = true;
  } else if (state.controls.left.pressed) {
    move_left();
    printf("left");
    inputDetected = true;
  } else if (state.controls.down.pressed) {
    move_down();
    printf("down");
    inputDetected = true;
  } else if (state.controls.right.pressed) {
    move_right();
    printf("right");
    inputDetected = true;
  } else if (state.controls.restart.pressed) {
    start();
    return;
  }

  update_game_progress();

  if (inputDetected && state.progress == NOT_OVER) {
    insert_new();
  }
}

void game_start() {
// //   init_serial();
// //   init_gdt();
// //   init_idt();
// //   init_pit(TIMER_HZ);
// //   init_rtc();
// //   init_screen(info);
// init_keyboard();

//   // Set random
  // datetime_t current = rtc_get_datetime();
  // srand(234);

  // // puts("Hello From OS!\n");
  // // rtc_write (NULL, 32,NULL);
  // start_game();

  // uint32_t last = pit_get_ticks();

  // while(1) {
  // //   // uint32_t now = pit_get_ticks();

  // //   // if ((now - last) > (TIMER_HZ / FPS)) {
  // //   //   last = now;

  //     update();
  //     render();

  // //     // screen_swap(&CURRENT);
  // //   // }
  // }
}


void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
               colour_t colour) {
  int i, j;
  for (i = x; i < x + w; i++) {
    for (j = y; j < y + h; j++) {
      set_pixel(i, j, get_colour(&colour));
    }
  }
}
uint32_t get_colour(colour_t *colour) {
  return (uint32_t)((colour->r << RED_SHIFT) | (colour->g << GREEN_SHIFT) |
                    (colour->b << BLUE_SHIFT));
}
void font_putc(char c, uint16_t x, uint16_t y, uint8_t scale,
               colour_t foreground, colour_t background) {
  uint8_t *glyph = &fb_font.data[c * fb_font.glyph_size];

  static const uint8_t masks[8] = {128, 64, 32, 16, 8, 4, 2, 1};

  uint16_t i, j;
  for (i = 0; i < fb_font.height; i++) {
    for (j = 0; j < fb_font.width; j++) {
      if (glyph[i] & masks[j]) {
        draw_rect(x + j * scale, y + i * scale, scale, scale, foreground);
      } else {
        draw_rect(x + j * scale, y + i * scale, scale, scale, background);
      }
    }
  }
}
void font_puts(char *string, uint16_t x, uint16_t y, uint8_t scale,
               colour_t foreground, colour_t background) {
  uint16_t cursor_x = x;
  while (*string) {
    font_putc(*string++, cursor_x, y, scale, foreground, background);
    cursor_x += fb_font.width * scale;
  }
}

void font_puts_center(char *string, uint16_t center_x, uint16_t center_y,
                      uint8_t scale, colour_t foreground, colour_t background) {
  unsigned int length = strlen(string);

  uint16_t x = center_x - ((length * fb_font.width * scale) / 2);
  uint16_t y = center_y - ((fb_font.height * scale) / 2);

  font_puts(string, x, y, scale, foreground, background);
}

