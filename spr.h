#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#define W 8
#define T 16
#define SZ (W * W * T * T)

#define MIN_LINES 10
#define MIN_COLS 64

typedef uint16_t coord_t;
typedef uint8_t color_t;

// TODO: don't need a reusable struct for worlds, only editing 1 file anyway!
// refactor into spr struct
struct world {
  color_t data[SZ];
};
void world_draw(struct world *world);
bool world_edit(struct world *world, int x, int y, int tx, int ty, color_t v);
int world_geti(int x, int y, int tx, int ty);
color_t world_get(struct world *world, int x, int y, int tx, int ty);
void world2chr(struct world *world);
void chr2world(struct world *world, char *file);

struct spr_ {
  struct world *world;
  char *name;
  bool quit, redraw, loop, edited;

  // (x,y) = pos, (tx,ty) = sprite, (cx, cy) = camera
  coord_t x, y, tx, ty, cx, cy;

  WINDOW *status, *draw[2][2], *out;
};
int spr_init(char *name);
void spr_destroy();
void spr_draw();
void spr_status();
int spr_save();
void spr_log(char *fmt, ...);

int curses_init();
void curses_destroy();
void curses_clear();
