#pragma once

#include "core/grid.h"

extern Grid back_buf;

typedef struct {
  int width, height;
} Viewport; // Assumed zeros on x and y.

typedef struct {
  int x, y;
  int width, height;
} Rect;

typedef struct {
  int is_running;
  Viewport viewport;
} App;

extern App app;

typedef enum {
  KEY_NONE,
  KEY_MENU,
  KEY_OBJECT_MENU,
  KEY_UP,
  KEY_DOWN,
  KEY_ENTER,
  KEY_ESCAPE,
} TuiKey;

void copy_back_to_front(void);
TuiKey input(void);
int clear_screen(void);
void restore_terminal(void);
int init_terminal(void);
int update_viewport(void);
int draw_screen(void);
void draw_rect(Rect rect);
