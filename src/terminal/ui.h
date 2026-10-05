#pragma once

#include "renderer/renderer.h"
#include "tui.h"

typedef enum {
  UI_MENU_NONE,
  UI_MENU_RENDERER,
  UI_MENU_OBJECT,
} UiMenu;

typedef struct {
  Backend selected;
  int highlighted;
  UiMenu menu;
  Grid cells;
} Ui;

typedef struct {
  double fps;
  double frame_ms;
} FrameStats;

void ui_handle_key(Ui *ui, TuiKey key);
Rect ui_render_area(int width, int height);
int ui_draw(Ui *ui, Grid *terminal, const FrameStats *stats);
void ui_free(Ui *ui);
