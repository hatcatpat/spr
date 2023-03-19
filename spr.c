#include "spr.h"
#include "config.h"

//================= MISC ======================
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BIT(x) (1 << (x))
#define GET_BIT(from, x) (((from) & (BIT((x)))) >> (x))

struct spr_ spr = {0};

//================= WORLD ======================
void world_init(struct world *world) {
  memset(world, 0, sizeof(struct world));

#ifdef DEBUG
  for (int i = 0; i < SZ; ++i)
    world->data[i] = rand() % 4;
#endif
}

void world_destroy(struct world *world) {
  memset(world, 0, sizeof(struct world));
}

int color2char(color_t color) {
  return (color ? (color + '1') : COLOR_0) | COLOR_PAIR(color + 1);
}

void world_draw(struct world *world) {
  for (int y = 0; y < W; ++y)
    for (int x = 0; x < W; ++x)
      waddch(spr.draw, color2char(world_get(world, x, y)) |
                           ((x == spr.x && y == spr.y) ? A_STANDOUT : 0));
}

int world_geti(int x, int y, int tx, int ty) {
  return (x + y * W) + (tx * W * W + ty * W * W * T);
}

bool world_edit(struct world *world, int x, int y, color_t v) {
  int I = world_geti(x, y, spr.tx, spr.ty);
  if (world->data[I] == v)
    return false;

  world->data[I] = v;
  return true;
}

color_t world_get(struct world *world, int x, int y) {
  if (x < W && y < W)
    return world->data[world_geti(x, y, spr.tx, spr.ty)];
  else
    return 0;
}

//================= SPR ======================
int spr_init(char *name) {
  spr.quit = false;
  spr.redraw = true;
  spr.loop = true;
  spr.edited = false;
  spr.name = name;
  spr.world = calloc(1, sizeof(struct world));

  if (!access(name, F_OK)) {
    if (!access(name, R_OK | W_OK))
      chr2world(spr.world, name);
    else
      return -1;
  } else
    world_init(spr.world);

  spr_draw();

  return 0;
}

void spr_destroy() {
  world_destroy(spr.world);
  free(spr.world);
}

char bool2char(bool b) { return b ? '+' : '-'; }
void spr_status() {
  uint8_t byte[2] = {0};
  for (int i = 0; i < 8; ++i) {
    color_t col = world_get(spr.world, i, spr.y);
    if (col == 0)
      continue;

    if (col == 1 || col == 3)
      byte[0] |= BIT(7 - i);

    if (col == 2 || col == 3)
      byte[1] |= BIT(7 - i);
  }

  wprintw(spr.info, "%s%c :: [%i,%i] :: (%i,%i) :: %%%c :: %02x+%02x", spr.name,
          bool2char(spr.edited), spr.tx, spr.ty, spr.x, spr.y,
          bool2char(spr.loop), byte[0], byte[1]);
}

void spr_draw() {
  if (!spr.redraw)
    return;

  clear();
  refresh();

  wclear(spr.info);
  spr_status();
  wrefresh(spr.info);

  wclear(spr.draw);
  world_draw(spr.world);
  wrefresh(spr.draw);

  spr.redraw = false;
}

int spr_save() {
  world2chr(spr.world);
  spr.edited = false;
  return 0;
}

void spr_log(char *fmt, ...) {
  va_list va;

  wclear(spr.out);
  va_start(va, fmt);
  vw_printw(spr.out, fmt, va);
  va_end(va);
  wrefresh(spr.out);
}

#define CHR_SZ (W * 2 * T * T)
void world2chr(struct world *world) {
  /* curses_clear(); */

  // * WRITING THE CHR FORMAT*
  //
  // the pixels (1 row of 8 pixels):
  // abcd efgh
  //
  // become 2 bytes, separated by 8 bytes (where each % is binary):
  // [left byte] %abcd %efgh
  // +8 bytes (for the other rows)
  // [right byte] %abcd %efgh
  //
  // if a color is 0, no bits will be enabled
  // if a color is 1, only the left bits will be enabled
  // if a color is 2, only the right bits will be enabled
  // if a color is 3, both bits will be enabled
  //
  // e.g.,
  // 1030 2111
  // [left]  %1010 %0111 = 128 + 32 + 4 + 2 + 1 = $a7
  // +8 bytes
  // [right] %0010 %1000 = 32 + 8 = $28
  //
  // data[i] = $a7, data[i + 8] = $28
  //
  uint8_t data[CHR_SZ] = {0};

  for (int s = 0; s < T * T; ++s) { // sprite
    for (int r = 0; r < 8; ++r) {   // row
      int S = s * 16 + r;

      data[S] = data[S + 8] = 0;

      for (int c = 0; c < 2; ++c) {   // col
        for (int i = 0; i < 4; ++i) { // pixel
          int I = (s * 64) + (8 * r) + (c * 4) + i;
          color_t col = world->data[I];
          if (col == 0)
            continue;

          if (col == 1 || col == 3)
            data[S] |= BIT(7 - (c * 4 + i));

          if (col == 2 || col == 3)
            data[S + 8] |= BIT(7 - (c * 4 + i));
        }
      }
    }
  }

  FILE *f = fopen(spr.name, "wb");
  if (!f) {
    spr_log("[error] unable to open file");
    return;
  }

  fwrite(data, sizeof(uint8_t), CHR_SZ, f);
  fclose(f);

  spr_log("saved :)");
}

void chr2world(struct world *world, char *file) {
  world_init(world);

  FILE *f = fopen(file, "rb");
  if (!f) {
    spr_log("[error] unable to read file\n");
    return;
  }

  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint8_t *data = calloc(MIN(sz, CHR_SZ), sizeof(uint8_t));
  fread(data, 1, sz, f);

  // * READING THE CHR FORMAT*
  // each byte corresponds to a row of 8 pixels
  // however, two of those bytes are required to fully describe the color
  //
  // the left byte and the right byte are seperated by 8 bytes
  // so, each sprite is made up of 16 bytes
  // and there are 16x16 of them
  //
  // the left/right bytes control color:
  // left = 0, right = 0 -> c = 0
  // left = 1, right = 0 -> c = 1
  // left = 0, right = 1 -> c = 2
  // left = 1, right = 1 -> c = 3
  //
  // e.g.,
  // the bytes: 1100 ... 0101 (seperated by 8 bytes)
  // gives non-zero pixels: xx_x
  // with colors: 1302
  //
  for (int s = 0; s < T * T; ++s) {        // sprites
    for (int r = 0; r < W; ++r) {          // row
      uint8_t L = data[s * W * 2 + r];     // left byte
      uint8_t R = data[s * W * 2 + r + 8]; // right byte
      for (int i = 0; i < 8; ++i)          // pixel
        world->data[world_geti(i, r, s, 0)] =
            GET_BIT(L, 7 - i) + 2 * GET_BIT(R, 7 - i);
    }
  }

  free(data);
  fclose(f);

  spr.redraw = true;
}

//================= CURSES ======================
int curses_init() {
  initscr();

  if (LINES < MIN_LINES || COLS < MIN_COLS)
    return 1;

  clear();
  refresh();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  curs_set(0);

  start_color();
  init_pair(1, COLOR_0_FG, COLOR_0_BG);
  init_pair(2, COLOR_1_FG, COLOR_1_BG);
  init_pair(3, COLOR_2_FG, COLOR_2_BG);
  init_pair(4, COLOR_3_FG, COLOR_3_BG);

  spr.info = newwin(1, 64, 0, 0);
  spr.draw = newwin(W, W, 1, 0);
  spr.out = newwin(1, 64, 1 + W, 0);

  return 0;
}

void curses_destroy() {
  delwin(spr.info), delwin(spr.draw), delwin(spr.out);
  endwin();
}

void curses_clear() {
  clear();
  refresh();
  wclear(spr.info);
  wrefresh(spr.info);
  wclear(spr.draw);
  wrefresh(spr.draw);
}

//================= MAIN ======================
int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (curses_init())
    goto CLEANUP;

  if (argc == 2) {
    if (spr_init(argv[1]))
      goto CLEANUP;
  } else {
    if (spr_init("untitled.chr"))
      goto CLEANUP;
  }

  int ch;
  while (!spr.quit) {
    ch = getch();

    switch (ch) {
      //================= MOVEMENT ======================
    case KEY_RIGHT:
      if (!spr.loop && spr.x == W - 1)
        goto TRIGHT;
      spr.x = (spr.x + 1) % W;
      spr.redraw = true;
      break;

    case KEY_LEFT:
      if (spr.x == 0) {
        if (spr.loop)
          spr.x = W - 1;
        else
          goto TLEFT;
      } else
        spr.x = (spr.x - 1) % W;

      spr.redraw = true;
      break;

    case KEY_DOWN:
      spr.y = (spr.y + 1) % W;
      spr.redraw = true;
      break;

    case KEY_UP:
      if (spr.y == 0)
        spr.y = W - 1;
      else
        spr.y = (spr.y - 1) % W;

      spr.redraw = true;
      break;

    case 'd':
    TRIGHT:
      spr.tx = (spr.tx + 1) % T;
      spr.redraw = true;
      break;

    case 'a':
    TLEFT:
      if (spr.tx == 0)
        spr.tx = T - 1;
      else
        spr.tx = (spr.tx - 1) % T;

      spr.redraw = true;
      break;

    case 's':
      spr.ty = (spr.ty + 1) % T;
      spr.redraw = true;
      break;

    case 'w':
      if (spr.ty == 0)
        spr.ty = T - 1;
      else
        spr.ty = (spr.ty - 1) % T;

      spr.redraw = true;
      break;
      //================= !MOVEMENT ======================

    case '1':
    case '2':
    case '3':
    case '4':
      spr.edited |= world_edit(spr.world, spr.x, spr.y, (ch - '1'));
      spr.redraw = true;
      break;

    case 'z':
      spr.loop = !spr.loop;
      spr.redraw = true;
      break;

    case '\n':
      spr_save();
      break;

    case 'r':
      chr2world(spr.world, spr.name);
      break;

    case 'q':
      spr.quit = true;
      break;
    }

    spr_draw();
  }

CLEANUP:
  spr_destroy();
  curses_destroy();

  return 0;
}
