#pragma once

typedef struct {
  float inverse_z;
  float brightness;
} FramebufferDot;

typedef struct {
  FramebufferDot *data;
  int width;
  int height;
} Framebuffer;

int framebuffer_resize(Framebuffer *buffer, int width, int height);
void framebuffer_clear(Framebuffer *buffer);

int framebuffer_write(Framebuffer *buffer, int x, int y, float inverse_z,
                      float brightness);
void framebuffer_free(Framebuffer *buffer);
