#pragma once

#include "renderer/render_math.h"

typedef enum {
  SHAPE_TORUS,
  SHAPE_CUBE,
  SHAPE_SPHERE,
  SHAPE_COUNT,
} Shape;

typedef struct {
  int a, b, c;
} Triangle;

typedef struct {
  const vec3 *vertices;
  const Triangle *triangles;
  int vertex_count;
  int triangle_count;
} Mesh;

const char *shapes_name(Shape shape);
Mesh shapes_get(Shape shape);
