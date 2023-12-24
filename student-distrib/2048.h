#ifndef SCREEN_H
#define SCREEN_H

// #include <boot/stivale2.h>
// #include <liblog/logging.h>
// #include <libmem/memory.h>
// #include <stddef.h>
// #include <stdint.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_BPP 32
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define BUFFER_SIZE SCREEN_SIZE *(SCREEN_BPP / sizeof(uint32_t))

#define RED_SHIFT 16
#define GREEN_SHIFT 8
#define BLUE_SHIFT 0

#define RGB(r, g, b) ((colour_t){r, g, b})
#define FONT_WIDTH 6
#define FONT_HEIGHT 12

struct psf {
  uint32_t magic;
  uint32_t version;
  uint32_t headersize;
  uint32_t flags;

  uint32_t numglyph;
  uint32_t glyph_size;
  uint32_t height;
  uint32_t width;

  uint8_t data[];
};

typedef struct {
  uint8_t r, g, b;
} colour_t;

// void screen_swap(void);
uint32_t get_colour(colour_t *colour);
void set_pixel(uint16_t x, uint16_t y, uint32_t colour);
void set_screen(uint32_t colour);
void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,colour_t colour);
uint32_t get_colour(colour_t *colour);
void font_putc(char c, uint16_t x, uint16_t y, uint8_t scale,colour_t foreground, colour_t background);
void font_puts(char *string, uint16_t x, uint16_t y, uint8_t scale, colour_t foreground, colour_t background);
void font_puts_center(char *string, uint16_t center_x, uint16_t center_y,uint8_t scale, colour_t foreground, colour_t background);
void update(void);
void start_game(void);
void render(void);
void render_loss(void);
void render_win(void);
void render_dialog(void);
void power(uint32_t inp,uint32_t vals);






// void init_screen(struct stivale2_struct *info);

#endif
