#pragma once

#include "core/grid.h"

typedef char BrailleCell[32];

typedef enum {
  RENDER_RASTERIZER,
  RENDER_RAYMARCHER,
} Backend;

int set_render_backend(Backend backend);
int renderer_object_count(Backend backend);
const char *renderer_object_name(Backend backend, int index);
void renderer_select_object(Backend backend, int index);
int renderer_selected_object(Backend backend);

int render(int width, int height, double time);
int generate_image(int width, int height);
Grid get_image(void);
void free_renderer(void);
