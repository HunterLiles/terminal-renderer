#include "renderer.h"

#include "framebuffer.h"
#include "lighting.h"
#include "rasterizer/rasterizer.h"
#include "raymarcher/raymarcher.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>

static Framebuffer render_buf;
static Grid braille_buf = {.elem_size = sizeof(BrailleCell)};
static Backend render_backend = RENDER_RASTERIZER;
static Shape selected_shape = SHAPE_TORUS;
static Sdf selected_sdf = SDF_DISPLACED_SPHERE;

static const Lighting lighting = {
    .direction_to_light = {-0.4f, 0.7f, -1.0f},
    .intensity = 0.8f,
    .ambient = 0.2f,
};

static int valid_dimensions(int width, int height) {
  if (width <= 0 || height <= 0 || width > INT_MAX / 2 ||
      height > INT_MAX / 4) {
    errno = EINVAL;
    return 0;
  }
  return 1;
}

int set_render_backend(Backend backend) {
  if (backend != RENDER_RASTERIZER && backend != RENDER_RAYMARCHER) {
    errno = EINVAL;
    return 0;
  }
  render_backend = backend;
  return 1;
}

int renderer_object_count(Backend backend) {
  switch (backend) {
  case RENDER_RASTERIZER:
    return SHAPE_COUNT;
  case RENDER_RAYMARCHER:
    return SDF_COUNT;
  default:
    return 0;
  }
}

const char *renderer_object_name(Backend backend, int index) {
  switch (backend) {
  case RENDER_RASTERIZER:
    return shapes_name((Shape)index);
  case RENDER_RAYMARCHER:
    return sdf_name((Sdf)index);
  default:
    return "";
  }
}

void renderer_select_object(Backend backend, int index) {
  switch (backend) {
  case RENDER_RASTERIZER:
    selected_shape = (Shape)index;
    break;
  case RENDER_RAYMARCHER:
    selected_sdf = (Sdf)index;
    break;
  default:
    break;
  }
}

int renderer_selected_object(Backend backend) {
  switch (backend) {
  case RENDER_RASTERIZER:
    return selected_shape;
  case RENDER_RAYMARCHER:
    return selected_sdf;
  default:
    return -1;
  }
}

int render(int width, int height, double time) {
  if (!valid_dimensions(width, height))
    return 0;
  if (!framebuffer_resize(&render_buf, width * 2, height * 4))
    return 0;
  framebuffer_clear(&render_buf);
  if (render_backend == RENDER_RASTERIZER) {
    rasterizer_render(&render_buf, &lighting, selected_shape, time);
  } else {
    raymarcher_render(&render_buf, &lighting, selected_sdf, time);
  }
  return 1;
}

static void encode_braille(unsigned int pattern, float brightness,
                           BrailleCell cell) {
  if (pattern == 0) {
    cell[0] = ' ';
    cell[1] = '\0';
    return;
  }

  unsigned int codepoint = 0x2800 + pattern;
  char character[4] = {
      (char)(224 + codepoint / 4096),
      (char)(128 + (codepoint / 64) % 64),
      (char)(128 + codepoint % 64),
      '\0',
  };

  if (brightness < 0.0f)
    brightness = 0.0f;
  if (brightness > 1.0f)
    brightness = 1.0f;
  int shade = 232 + (int)(brightness * 23);
  snprintf(cell, sizeof(BrailleCell), "\x1b[38;5;%dm%s\x1b[0m", shade,
           character);
}

int generate_image(int width, int height) {
  if (!valid_dimensions(width, height))
    return 0;
  if (render_buf.data == NULL || render_buf.width != width * 2 ||
      render_buf.height != height * 4) {
    errno = EINVAL;
    return 0;
  }
  if (!grid_resize(&braille_buf, width, height))
    return 0;

  const FramebufferDot *render = render_buf.data;
  BrailleCell *braille = braille_buf.data;
  static const unsigned int dot_values[8] = {1, 8, 2, 16, 4, 32, 64, 128};
  size_t cell_count = (size_t)width * height;

  for (size_t cell = 0; cell < cell_count; cell++) {
    size_t x = cell % width;
    size_t y = cell / width;
    unsigned int pattern = 0;
    float brightness_sum = 0.0f;
    int active_dots = 0;

    for (int dot = 0; dot < 8; dot++) {
      int dot_x = dot % 2;
      int dot_y = dot / 2;
      size_t index = (y * 4 + dot_y) * render_buf.width + x * 2 + dot_x;
      if (render[index].inverse_z > 0.0f) {
        pattern += dot_values[dot];
        brightness_sum += render[index].brightness;
        active_dots++;
      }
    }
    float brightness = active_dots ? brightness_sum / active_dots : 0.0f;
    encode_braille(pattern, brightness, braille[cell]);
  }
  return 1;
}

Grid get_image(void) { return braille_buf; }

void free_renderer(void) {
  framebuffer_free(&render_buf);
  grid_free(&braille_buf);
  selected_shape = SHAPE_TORUS;
  selected_sdf = SDF_DISPLACED_SPHERE;
  render_backend = RENDER_RASTERIZER;
}
