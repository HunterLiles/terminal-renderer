#include "grid.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

int grid_resize(Grid *grid, int width, int height) {
  if (width <= 0 || height <= 0 || grid->elem_size == 0) {
    errno = EINVAL;
    return 0;
  }
  if ((size_t)width > SIZE_MAX / (size_t)height ||
      (size_t)width * height > SIZE_MAX / grid->elem_size) {
    errno = ENOMEM;
    return 0;
  }
  if (grid->width == width && grid->height == height)
    return 1;

  void *data = calloc((size_t)width * (size_t)height, grid->elem_size);
  if (data == NULL)
    return 0;

  free(grid->data);
  grid->data = data;
  grid->width = width;
  grid->height = height;
  return 1;
}

void grid_free(Grid *grid) {
  free(grid->data);
  grid->data = NULL;
  grid->width = 0;
  grid->height = 0;
}
