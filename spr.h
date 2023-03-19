#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ncurses.h>

//================= TYPEDEFS ======================
typedef uint16_t coord_t;
typedef uint8_t byte_t;

//================= SPR ======================
#define SZ (2 * 8 * 16 * 16)
struct spr_ {
  byte_t data[SZ];
  char *name;
  bool quit, redraw, loop, edited;
  coord_t x, y, spx, spy, cx, cy;
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
bool spr_set(int x, int y, int spx, int spy, byte_t c);
int spr_geti(int x, int y, int spx, int spy);
byte_t spr_get(int x, int y, int spx, int spy);

//================= CURSES ======================
#define MIN_LINES (1 + 8 + 1 + 8 + 1)
#define MIN_COLS 64
int curses_init();
void curses_destroy();
void curses_clear();
