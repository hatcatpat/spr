#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ncurses.h>

#define W 8
#define T 16
#define SZ (W * W * T * T)

#define MIN_LINES (1 + 8 + 1 + 8 + 1)
#define MIN_COLS 64

typedef uint16_t coord_t;
typedef uint8_t color_t;

struct spr_ {
  color_t data[SZ];
  char *name;
  bool quit, redraw, loop, edited;
  coord_t x, y, tx, ty, cx, cy;
  WINDOW *status, *draw[2][2], *out;
};
int spr_init(char *name);
void spr_destroy();
void spr_draw();
void spr_status();
int spr_save();
int spr_load();
void spr_log(char *fmt, ...);

void spr_draw_data();
bool spr_edit(int x, int y, int tx, int ty, color_t v);
int spr_geti(int x, int y, int tx, int ty);
color_t spr_get(int x, int y, int tx, int ty);

int curses_init();
void curses_destroy();
void curses_clear();
