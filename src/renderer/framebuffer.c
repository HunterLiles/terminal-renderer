#include "framebuffer.h"

#include "core/grid.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

int framebuffer_resize(Framebuffer *buffer, int width, int height) {
  Grid storage = {
      .data = buffer->data,
      .width = buffer->width,
      .height = buffer->height,
      .elem_size = sizeof(FramebufferDot),
  };
  if (!grid_resize(&storage, width, height))
    return 0;

  buffer->data = storage.data;
  buffer->width = storage.width;
  buffer->height = storage.height;
  return 1;
}

void framebuffer_clear(Framebuffer *buffer) {
  if (buffer->data != NULL)
    memset(buffer->data, 0,
           (size_t)buffer->width * buffer->height * sizeof(FramebufferDot));
}

int framebuffer_write(Framebuffer *buffer, int x, int y, float inverse_z,
                      float brightness) {
  if (buffer->data == NULL || x < 0 || y < 0 || x >= buffer->width ||
      y >= buffer->height || !isfinite(inverse_z) || inverse_z <= 0.0f ||
      !isfinite(brightness))
    return 0;

  size_t index = (size_t)y * buffer->width + x;
  FramebufferDot *dot = &buffer->data[index];
  if (inverse_z <= dot->inverse_z)
    return 0;

  dot->inverse_z = inverse_z;
  dot->brightness = brightness;
  return 1;
}

void framebuffer_free(Framebuffer *buffer) {
  free(buffer->data);
  *buffer = (Framebuffer){0};
}
