#include "spr.h"
#include "config.h"

//================= MISC ======================
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BIT(x) (1 << (x))
#define GET_BIT(from, x) (((from) & (BIT((x)))) >> (x))
#define ENABLE_BIT(from, x) ((from) | BIT(x))
#define DISABLE_BIT(from, x) ((from) & ~BIT(x))

struct spr_ spr = {0};

//=============================================
int color2char(byte_t color) {
  if (spr.numbers)
    return (color ? (color + '1') : COLOR_0) | COLOR_PAIR(color + 1);
  else
    return (color ? ' ' : COLOR_0) | COLOR_PAIR(color + 1);
}

void spr_draw_data() {
  for (int Y = 0; Y < 2; ++Y) {
    for (int X = 0; X < 2; ++X) {
      for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
          int i = (spr.cx + X) % 16;
          int j = (spr.cy + Y) % 16;
          bool hover =
              (i == spr.spx && j == spr.spy) && (x == spr.x && y == spr.y);
          int attr = hover ? A_STANDOUT : 0;
          waddch(spr.draw[X][Y], color2char(spr_get(x, y, i, j)) | attr);
        }
      }
    }
  }
}

int spr_geti(int y, int spx, int spy) { return y + spx * 16 + spy * 16 * 16; }

byte_t spr_get(int x, int y, int spx, int spy) {
  int I = spr_geti(y, spx, spy);
  return GET_BIT(spr.data[I], 7 - x) + 2 * GET_BIT(spr.data[I + 8], 7 - x);
}

bool spr_set(int x, int y, int spx, int spy, byte_t c) {
  if (c == spr_get(x, y, spx, spy))
    return false;

  int I = spr_geti(y, spx, spy);
  byte_t b = BIT(7 - x);
  switch (c) {
  case 0:
    spr.data[I] &= ~b;
    spr.data[I + 8] &= ~b;
    break;

  case 1:
    spr.data[I] |= b;
    spr.data[I + 8] &= ~b;
    break;

  case 2:
    spr.data[I] &= ~b;
    spr.data[I + 8] |= b;
    break;

  case 3:
    spr.data[I] |= b;
    spr.data[I + 8] |= b;
    break;
  }

  return true;
}

int spr_init(char *name) {
  spr.quit = false;
  spr.redraw = true;
  spr.loop = false;
  spr.edited = false;
  spr.numbers = SHOW_NUMBERS;
  spr.fill = false;
  spr.last = 0;
  spr.name = name;

  if (!access(name, F_OK)) {
    if (!access(name, R_OK | W_OK))
      spr_load();
    else
      return -1;
  }

  spr_draw();

  return 0;
}

char bool2char(bool b) { return b ? '+' : '-'; }
void spr_status() {
  int I = spr_geti(spr.y, spr.spx, spr.spy);
  byte_t b[2] = {spr.data[I], spr.data[I + 8]};
  byte_t cur = spr_get(spr.x, spr.y, spr.spx, spr.spy);
  wprintw(spr.status,
          "%s%c :: [%i,%i] :: (%i,%i) :: %c :: %02x+%02x :: %%%c f%c%i",
          spr.name, bool2char(spr.edited), spr.spx, spr.spy, spr.x, spr.y,
          cur == 0 ? COLOR_0 : ((cur + 1) + '0'), b[0], b[1],
          bool2char(spr.loop), bool2char(spr.fill), spr.last + 1);
}

void spr_draw() {
  if (!spr.redraw)
    return;

  wclear(spr.status);
  spr_status();
  wrefresh(spr.status);

  for (int X = 0; X < 2; X++) {
    for (int Y = 0; Y < 2; Y++) {
      wclear(spr.draw[X][Y]);
      spr_draw_data();
      wrefresh(spr.draw[X][Y]);
    }
  }

  spr.redraw = false;
}

void spr_log(char *fmt, ...) {
  va_list va;

  wclear(spr.out);
  va_start(va, fmt);
  vw_printw(spr.out, fmt, va);
  va_end(va);
  wrefresh(spr.out);
}

int spr_load() {
  FILE *f = fopen(spr.name, "rb");
  if (!f) {
    spr_log("[error] unable to read file\n");
    return 1;
  }

  fseek(f, 0, SEEK_END);
  long sz = MIN(ftell(f), SZ);
  fseek(f, 0, SEEK_SET);

  memset(spr.data, 0, SZ * sizeof(byte_t));
  fread(spr.data, 1, sz, f);

  fclose(f);

  spr.edited = false;
  spr.redraw = true;
  spr_draw();
  spr_log("loaded :)");

  return 0;
}

int spr_save() {
  FILE *f = fopen(spr.name, "wb");
  if (!f) {
    spr_log("[error] unable to open file");
    return 1;
  }

  fwrite(spr.data, sizeof(byte_t), SZ, f);

  fclose(f);

  spr.edited = false;
  spr.redraw = true;
  spr_draw();

  spr_log("saved :)");

  return 0;
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
#ifdef MOUSE
  mousemask(BUTTON1_CLICKED, NULL);
#endif

  start_color();

#ifdef RGB
  init_color(COLOR_BLACK, BLACK);
  init_color(COLOR_RED, RED);
  init_color(COLOR_GREEN, GREEN);
  init_color(COLOR_YELLOW, YELLOW);
  init_color(COLOR_BLUE, BLUE);
  init_color(COLOR_MAGENTA, MAGENTA);
  init_color(COLOR_CYAN, CYAN);
  init_color(COLOR_WHITE, WHITE);
#endif

  init_pair(1, COLOR_0_FG, COLOR_0_BG);
  init_pair(2, COLOR_1_FG, COLOR_1_BG);
  init_pair(3, COLOR_2_FG, COLOR_2_BG);
  init_pair(4, COLOR_3_FG, COLOR_3_BG);

  spr.status = newwin(1, 64, 0, 0);
  for (int x = 0; x < 2; x++)
    for (int y = 0; y < 2; y++)
      spr.draw[x][y] = newwin(8, 8, 1 + y * (8 + 1), x * (8 + 1));

  spr.out = newwin(1, 64, 2 * (8 + 1), 0);

  return 0;
}

void curses_destroy() {
  delwin(spr.status), delwin(spr.out);
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
      delwin(spr.draw[i][j]);
  endwin();
}

void curses_clear() {
  clear();
  refresh();
  wclear(spr.status);
  wrefresh(spr.status);
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
      wclear(spr.draw[i][j]), wrefresh(spr.draw[i][j]);
}

//================= MAIN ======================
int main(int argc, char *argv[]) {
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
#ifdef MOUSE
  MEVENT evt;
#endif
  while (!spr.quit) {
    ch = getch();

    switch (ch) {
#ifdef MOUSE
      //================= MOUSE ======================
    case KEY_MOUSE:
      if ((getmouse(&evt) == OK) && (evt.bstate & BUTTON1_CLICKED)) {
        for (int X = 0; X < 2; ++X)
          for (int Y = 0; Y < 2; ++Y)
            if (wenclose(spr.draw[X][Y], evt.y, evt.x)) {
              int x, y;
              getbegyx(spr.draw[X][Y], y, x);
              spr.x = evt.x - x, spr.y = evt.y - y;
              spr.spx = (spr.cx + X) % 16, spr.spy = (spr.cy + Y) % 16;
              spr.redraw = true;
            }
      }
      break;
#endif

      //================= MOVEMENT ======================
      // move cursor
    case KEY_RIGHT:
      if (!spr.loop && spr.x == 8 - 1) {
        spr.x = 0;
        goto TRIGHT;
      }
      spr.x = (spr.x + 1) % 8;

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;

    case KEY_LEFT:
      if (spr.x == 0) {
        spr.x = 8 - 1;
        if (!spr.loop)
          goto TLEFT;
      } else
        spr.x = (spr.x - 1) % 8;

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;

    case KEY_DOWN:
      if (!spr.loop && spr.y == 8 - 1) {
        spr.y = 0;
        goto TDOWN;
      }
      spr.y = (spr.y + 1) % 8;

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;

    case KEY_UP:
      if (spr.y == 0) {
        spr.y = 8 - 1;
        if (!spr.loop)
          goto TUP;
      } else
        spr.y = (spr.y - 1) % 8;

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;

      // move sprite
    case 'd':
    TRIGHT:
      spr.spx = (spr.spx + 1) % 16;
      if (spr.spx == 0)
        spr.cx = 16 - 1;
      else if (spr.cx == 16 - 1 && spr.spx == 1)
        spr.cx = 0;
      else if (spr.spx > spr.cx + 1)
        spr.cx = (spr.cx + 1) % 16;

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;

    case 'a':
    TLEFT:
      if (spr.spx == 0)
        spr.spx = spr.cx = 16 - 1;
      else {
        spr.spx--;
        if (spr.spx < spr.cx)
          spr.cx--;
      }

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;

    case 's':
    TDOWN:
      spr.spy = (spr.spy + 1) % 16;
      if (spr.spy == 0)
        spr.cy = 16 - 1;
      else if (spr.cy == 16 - 1 && spr.spy == 1)
        spr.cy = 0;
      else if (spr.spy > spr.cy + 1)
        spr.cy = (spr.cy + 1) % 16;

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;

    case 'w':
    TUP:
      if (spr.spy == 0)
        spr.spy = spr.cy = 16 - 1;
      else {
        spr.spy--;
        if (spr.spy < spr.cy)
          spr.cy--;
      }

      if (spr.fill)
        goto SET;

      spr.redraw = true;
      break;
      //================= !MOVEMENT ======================

    case '1':
    case '2':
    case '3':
    case '4': // set color
      spr.last = ch - '1';
    SET:
      spr.edited |= spr_set(spr.x, spr.y, spr.spx, spr.spy, spr.last);
      spr.redraw = true;
      break;

    case '%': // toggle loop
      spr.loop = !spr.loop;
      spr.redraw = true;
      break;

    case '`': // toggle numbers
      spr.numbers = !spr.numbers;
      spr.redraw = true;
      break;

    case '\n': // save
      spr_save();
      break;

    case 'r': // load
      spr_load();
      break;

    case 'f': // toggle fill-mode
      spr.fill = !spr.fill;
      spr.redraw = true;
      break;

    case KEY_DC: // delete sprite
      for (int r = 0; r < 8; ++r) {
        int I = spr_geti(r, spr.spx, spr.spy);
        spr.edited |= spr.data[I] | spr.data[I];
        spr.data[I] = spr.data[I + 8] = 0;
      }
      spr.redraw = true;
      break;

    case 'q': // quit
      spr.quit = true;
      break;
    }

    spr_draw();
  }

CLEANUP:
  curses_destroy();

  return 0;
}
