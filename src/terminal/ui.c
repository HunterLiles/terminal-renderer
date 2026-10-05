#include "ui.h"

#include <stdio.h>
#include <string.h>

typedef char UiCell[32];

enum {
  HEADER_HEIGHT = 3,
  OBJECT_X = 29,
  MENU_WIDTH = 22,
  RENDERER_COUNT = 2,
};

static int header_height(int height) {
  return height < HEADER_HEIGHT ? height : HEADER_HEIGHT;
}

static const char *renderer_label(Backend backend) {
  if (backend == RENDER_RASTERIZER) {
    return "Rasterizer";
  }
  return "Raymarcher";
}

static int menu_count(const Ui *ui) {
  if (ui->menu == UI_MENU_RENDERER) {
    return RENDERER_COUNT;
  }
  return renderer_object_count(ui->selected);
}

void ui_handle_key(Ui *ui, TuiKey key) {
  if (key == KEY_MENU || key == KEY_OBJECT_MENU) {
    UiMenu menu = UI_MENU_RENDERER;
    if (key == KEY_OBJECT_MENU) {
      menu = UI_MENU_OBJECT;
    }

    if (ui->menu == menu) {
      ui->menu = UI_MENU_NONE;
    } else {
      ui->menu = menu;
      if (menu == UI_MENU_RENDERER) {
        ui->highlighted = ui->selected;
      } else {
        ui->highlighted = renderer_selected_object(ui->selected);
      }
    }
    return;
  }

  if (ui->menu == UI_MENU_NONE) {
    return;
  }

  int count = menu_count(ui);
  switch (key) {
  case KEY_UP:
    ui->highlighted--;
    if (ui->highlighted < 0) {
      ui->highlighted = count - 1;
    }
    break;
  case KEY_DOWN:
    ui->highlighted++;
    if (ui->highlighted == count) {
      ui->highlighted = 0;
    }
    break;
  case KEY_ENTER:
    if (ui->menu == UI_MENU_RENDERER) {
      Backend backend = (Backend)ui->highlighted;
      if (set_render_backend(backend)) {
        ui->selected = backend;
      }
    } else {
      renderer_select_object(ui->selected, ui->highlighted);
    }
    ui->menu = UI_MENU_NONE;
    break;
  case KEY_ESCAPE:
    ui->menu = UI_MENU_NONE;
    break;
  default:
    break;
  }
}

Rect ui_render_area(int width, int height) {
  int header = header_height(height);
  return (Rect){.x = 0, .y = header, .width = width, .height = height - header};
}

static void put_cell(Ui *ui, Grid *terminal, int x, int y, const char *text) {
  if (x < 0 || y < 0 || x >= terminal->width || y >= terminal->height)
    return;
  size_t index = (size_t)y * terminal->width + x;
  UiCell *cells = ui->cells.data;
  snprintf(cells[index], sizeof(UiCell), "%s", text);
  const char **back = terminal->data;
  back[index] = cells[index];
}

static void put_text(Ui *ui, Grid *terminal, int x, int y, const char *text,
                     int highlighted, int max_width) {
  for (int i = 0; text[i] && i < max_width && x + i < terminal->width; i++) {
    char cell[32];
    if (highlighted)
      snprintf(cell, sizeof(cell), "\x1b[7m%c\x1b[0m", text[i]);
    else
      snprintf(cell, sizeof(cell), "%c", text[i]);
    put_cell(ui, terminal, x + i, y, cell);
  }
}

static void horizontal(Ui *ui, Grid *terminal, int y, int x, int width,
                       const char *left, const char *right) {
  for (int i = 0; i < width; i++) {
    const char *glyph = "─";
    if (i == 0) {
      glyph = left;
    } else if (i == width - 1) {
      glyph = right;
    }
    put_cell(ui, terminal, x + i, y, glyph);
  }
}

static void draw_header(Ui *ui, Grid *terminal, const FrameStats *stats) {
  int width = terminal->width;
  int height = terminal->height;
  for (int y = 0; y < header_height(height); y++)
    for (int x = 0; x < width; x++)
      put_cell(ui, terminal, x, y, " ");

  horizontal(ui, terminal, 0, 0, width, "┌", "┐");
  if (height > 1) {
    put_cell(ui, terminal, 0, 1, "│");
    char label[80];
    snprintf(label, sizeof(label), "Renderer: [ %s v ]",
             renderer_label(ui->selected));
    put_text(ui, terminal, 2, 1, label, ui->menu == UI_MENU_RENDERER,
             width - 3);
    int selected_object = renderer_selected_object(ui->selected);
    const char *object_name =
        renderer_object_name(ui->selected, selected_object);
    snprintf(label, sizeof(label), "Object: [ %s v ]", object_name);
    put_text(ui, terminal, OBJECT_X, 1, label, ui->menu == UI_MENU_OBJECT,
             width - OBJECT_X - 1);
    char text[48];
    snprintf(text, sizeof(text), "%.0f FPS | %.1f ms", stats->fps,
             stats->frame_ms);
    int length = (int)strlen(text);
    int stats_x = width - 2 - length;
    int selector_end = OBJECT_X + (int)strlen(label);
    if (stats_x < selector_end + 2) {
      snprintf(text, sizeof(text), "%.0f FPS", stats->fps);
      length = (int)strlen(text);
      stats_x = width - 2 - length;
    }
    int help_end = width - 2;
    if (stats_x >= selector_end + 2) {
      put_text(ui, terminal, stats_x, 1, text, 0, length);
      help_end = stats_x - 2;
    }
    const char *help = "r: renderer  o: object  q: quit";
    if (ui->menu != UI_MENU_NONE) {
      help = "j/k or arrows: move  Enter: select  Esc: back";
    }
    int help_x = selector_end + 2;
    if (help_end - help_x < (int)strlen(help) && ui->menu != UI_MENU_NONE)
      help = "j/k Enter Esc";
    if (help_end - help_x >= (int)strlen(help))
      put_text(ui, terminal, help_x, 1, help, 0, help_end - help_x);
    put_cell(ui, terminal, width - 1, 1, "│");
  }
  if (height > 2)
    horizontal(ui, terminal, 2, 0, width, "└", "┘");
}

static void draw_dropdown(Ui *ui, Grid *terminal) {
  int width = terminal->width;
  int height = terminal->height;
  if (ui->menu == UI_MENU_NONE || width <= 0 || height <= 0)
    return;

  int menu_width = MENU_WIDTH;
  if (menu_width > width) {
    menu_width = width;
  }
  int x = 12;
  if (ui->menu == UI_MENU_OBJECT) {
    x = OBJECT_X + 8;
  }
  if (x + menu_width > width)
    x = width - menu_width;
  int top = HEADER_HEIGHT;
  if (height <= HEADER_HEIGHT) {
    top = 0;
  }
  int available = height - top;
  int border = available >= 3 && menu_width >= 3;
  int rows = available;
  if (border) {
    rows -= 2;
  }
  int count = menu_count(ui);
  if (rows > count)
    rows = count;
  int first = 0;
  if (ui->highlighted >= rows) {
    first = ui->highlighted - rows + 1;
  }
  if (border)
    horizontal(ui, terminal, top, x, menu_width, "┌", "┐");
  for (int row = 0; row < rows; row++) {
    int option = first + row;
    int y = top + border + row;
    for (int i = 0; i < menu_width; i++)
      put_cell(ui, terminal, x + i, y, " ");
    const char *label;
    if (ui->menu == UI_MENU_RENDERER) {
      label = renderer_label((Backend)option);
    } else {
      label = renderer_object_name(ui->selected, option);
    }
    put_text(ui, terminal, x + border, y, label, ui->highlighted == option,
             menu_width - 2 * border);
    if (border) {
      put_cell(ui, terminal, x, y, "│");
      put_cell(ui, terminal, x + menu_width - 1, y, "│");
    }
  }
  if (border)
    horizontal(ui, terminal, top + rows + 1, x, menu_width, "└", "┘");
}

int ui_draw(Ui *ui, Grid *terminal, const FrameStats *stats) {
  ui->cells.elem_size = sizeof(UiCell);
  if (!grid_resize(&ui->cells, terminal->width, terminal->height))
    return 0;

  draw_header(ui, terminal, stats);
  draw_dropdown(ui, terminal);
  return 1;
}

void ui_free(Ui *ui) { grid_free(&ui->cells); }
