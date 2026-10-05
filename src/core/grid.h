#pragma once

#include <stddef.h>

typedef struct {
  void *data;
  int width;
  int height;
  size_t elem_size;
} Grid;

int grid_resize(Grid *grid, int width, int height);
void grid_free(Grid *grid);
