#include "rasterizer.h"

#include <math.h>

// Based on Tsoding's 3D renderer walkthrough:
// https://www.youtube.com/watch?v=qjWkNZ0SXfo&list=LL&index=1&t=1084s

static double edge_function(vec3 a, vec3 b, double x, double y) {
  return ((double)b.x - a.x) * (y - a.y) - ((double)b.y - a.y) * (x - a.x);
}

static float triangle_brightness(vec3 a, vec3 b, vec3 c,
                                 const Lighting *lighting) {
  vec3 ab = vec3_sub(b, a);
  vec3 ac = vec3_sub(c, a);
  SurfaceSample surface = {
      .position = vec3_scale(vec3_add(vec3_add(a, b), c), 1.0f / 3.0f),
      .normal = vec3_cross(ab, ac),
      .albedo = 1.0f,
  };
  return lighting_shade(&surface, lighting);
}

static void draw_triangle(Framebuffer *target, vec3 a, vec3 b, vec3 c,
                          float brightness) {
  if (!isfinite(a.x) || !isfinite(a.y) || !isfinite(a.z) || !isfinite(b.x) ||
      !isfinite(b.y) || !isfinite(b.z) || !isfinite(c.x) || !isfinite(c.y) ||
      !isfinite(c.z))
    return;

  double area = edge_function(a, b, c.x, c.y);
  if (area == 0)
    return;

  double left = fmin(a.x, b.x);
  left = fmin(left, c.x);
  double right = fmax(a.x, b.x);
  right = fmax(right, c.x);
  double top = fmin(a.y, b.y);
  top = fmin(top, c.y);
  double bottom = fmax(a.y, b.y);
  bottom = fmax(bottom, c.y);

  if (left < 0)
    left = 0;
  if (right > target->width - 1)
    right = target->width - 1;
  if (top < 0)
    top = 0;
  if (bottom > target->height - 1)
    bottom = target->height - 1;
  if (left > right || top > bottom)
    return;

  int min_x = (int)floor(left);
  int max_x = (int)ceil(right);
  int min_y = (int)floor(top);
  int max_y = (int)ceil(bottom);

  for (int y = min_y; y <= max_y; y++) {
    for (int x = min_x; x <= max_x; x++) {
      double weight_a = edge_function(b, c, x, y) / area;
      double weight_b = edge_function(c, a, x, y) / area;
      double weight_c = edge_function(a, b, x, y) / area;
      if (weight_a < 0 || weight_b < 0 || weight_c < 0)
        continue;

      float inverse_z =
          (float)(weight_a * a.z + weight_b * b.z + weight_c * c.z);
      framebuffer_write(target, x, y, inverse_z, brightness);
    }
  }
}

static vec3 screen(vec3 p, int width, int height) {
  return (vec3){.x = (p.x + 1) / 2 * (width - 1),
                .y = (1 - (p.y + 1) / 2) * (height - 1),
                .z = p.z};
}

static vec3 projection(vec3 p, float aspect) {
  return (vec3){
      .x = p.x / p.z / aspect,
      .y = p.y / p.z,
      .z = 1.0f / p.z,
  };
}

void rasterizer_render(Framebuffer *target, const Lighting *lighting,
                       Shape shape, double time) {
  Mesh mesh = shapes_get(shape);
  vec3 world_vertices[mesh.vertex_count];
  vec3 projected_vertices[mesh.vertex_count];
  int visible[mesh.vertex_count];

  vec3 move = {.x = 0, .y = 0, .z = 3};
  float aspect = (float)target->width / target->height;
  double rotation_cos = cos(time);
  double rotation_sin = sin(time);

  for (int i = 0; i < mesh.vertex_count; i++) {
    vec3 vertex = mesh.vertices[i];
    vec3 rotated = {
        .x = vertex.x * rotation_cos + vertex.z * rotation_sin,
        .y = vertex.y,
        .z = -vertex.x * rotation_sin + vertex.z * rotation_cos,
    };
    world_vertices[i] = vec3_add(rotated, move);
    vec3 p = world_vertices[i];
    visible[i] = isfinite(p.z) && p.z > 0.01f;
    if (visible[i]) {
      p = projection(p, aspect);
      p = screen(p, target->width, target->height);
      projected_vertices[i] = p;
    }
  }
  for (int i = 0; i < mesh.triangle_count; i++) {
    Triangle triangle = mesh.triangles[i];
    if (visible[triangle.a] && visible[triangle.b] && visible[triangle.c]) {
      float brightness = triangle_brightness(
          world_vertices[triangle.a], world_vertices[triangle.b],
          world_vertices[triangle.c], lighting);
      draw_triangle(target, projected_vertices[triangle.a],
                    projected_vertices[triangle.b],
                    projected_vertices[triangle.c], brightness);
    }
  }
}
