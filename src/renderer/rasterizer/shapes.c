#include "shapes.h"

enum {
  RING_SEGMENTS = 24,
  TUBE_SEGMENTS = 12,
  SPHERE_STACKS = 12,
  TORUS_VERTEX_COUNT = RING_SEGMENTS * TUBE_SEGMENTS,
  TORUS_TRIANGLE_COUNT = TORUS_VERTEX_COUNT * 2,
  SPHERE_VERTEX_COUNT = (SPHERE_STACKS - 1) * RING_SEGMENTS + 2,
  SPHERE_TRIANGLE_COUNT = (SPHERE_STACKS - 1) * RING_SEGMENTS * 2,
};

static const float pi = 3.14159265359f;

static const vec3 cube_vertices[] = {
    {-0.8f, -0.8f, -0.8f},
    {0.8f, -0.8f, -0.8f},
    {0.8f, 0.8f, -0.8f},
    {-0.8f, 0.8f, -0.8f},
    {-0.8f, -0.8f, 0.8f},
    {0.8f, -0.8f, 0.8f},
    {0.8f, 0.8f, 0.8f},
    {-0.8f, 0.8f, 0.8f},
};

static const Triangle cube_triangles[] = {
    {0, 2, 1},
    {0, 3, 2},
    {4, 5, 6},
    {4, 6, 7},
    {0, 4, 7},
    {0, 7, 3},
    {1, 2, 6},
    {1, 6, 5},
    {0, 1, 5},
    {0, 5, 4},
    {3, 7, 6},
    {3, 6, 2},
};

static vec3 torus_vertices[TORUS_VERTEX_COUNT];
static Triangle torus_triangles[TORUS_TRIANGLE_COUNT];
static int torus_generated;

static vec3 sphere_vertices[SPHERE_VERTEX_COUNT];
static Triangle sphere_triangles[SPHERE_TRIANGLE_COUNT];
static int sphere_generated;

static void generate_torus(void) {
  const float major_radius = 1.0f;
  const float tube_radius = 0.5f;
  const float tilt = 0.6f;
  float tilt_cos = cosf(tilt);
  float tilt_sin = sinf(tilt);

  for (int ring = 0; ring < RING_SEGMENTS; ring++) {
    float ring_angle = 2.0f * pi * ring / RING_SEGMENTS;
    int next_ring = (ring + 1) % RING_SEGMENTS;

    for (int tube = 0; tube < TUBE_SEGMENTS; tube++) {
      float tube_angle = 2.0f * pi * tube / TUBE_SEGMENTS;
      float radius = major_radius + tube_radius * cosf(tube_angle);
      vec3 point = {
          .x = radius * cosf(ring_angle),
          .y = tube_radius * sinf(tube_angle),
          .z = radius * sinf(ring_angle),
      };

      int next_tube = (tube + 1) % TUBE_SEGMENTS;
      int top_left = ring * TUBE_SEGMENTS + tube;
      int top_right = ring * TUBE_SEGMENTS + next_tube;
      int bottom_right = next_ring * TUBE_SEGMENTS + next_tube;
      int bottom_left = next_ring * TUBE_SEGMENTS + tube;

      torus_vertices[top_left] = (vec3){
          .x = point.x,
          .y = point.y * tilt_cos - point.z * tilt_sin,
          .z = point.y * tilt_sin + point.z * tilt_cos,
      };

      int triangle = top_left * 2;
      torus_triangles[triangle] =
          (Triangle){top_left, top_right, bottom_right};
      torus_triangles[triangle + 1] =
          (Triangle){top_left, bottom_right, bottom_left};
    }
  }
}

static void generate_sphere(void) {
  int bottom_pole = SPHERE_VERTEX_COUNT - 1;
  sphere_vertices[0] = (vec3){0, 1, 0};
  sphere_vertices[bottom_pole] = (vec3){0, -1, 0};

  for (int stack = 1; stack < SPHERE_STACKS; stack++) {
    float latitude = pi * stack / SPHERE_STACKS;
    float radius = sinf(latitude);

    for (int segment = 0; segment < RING_SEGMENTS; segment++) {
      float longitude = 2.0f * pi * segment / RING_SEGMENTS;
      int vertex = 1 + (stack - 1) * RING_SEGMENTS + segment;
      sphere_vertices[vertex] = (vec3){
          .x = radius * cosf(longitude),
          .y = cosf(latitude),
          .z = radius * sinf(longitude),
      };
    }
  }

  int triangle = 0;
  int last_ring = 1 + (SPHERE_STACKS - 2) * RING_SEGMENTS;

  for (int segment = 0; segment < RING_SEGMENTS; segment++) {
    int next_segment = (segment + 1) % RING_SEGMENTS;
    sphere_triangles[triangle++] =
        (Triangle){0, 1 + next_segment, 1 + segment};

    for (int stack = 0; stack < SPHERE_STACKS - 2; stack++) {
      int top_left = 1 + stack * RING_SEGMENTS + segment;
      int top_right = 1 + stack * RING_SEGMENTS + next_segment;
      int bottom_right = top_right + RING_SEGMENTS;
      int bottom_left = top_left + RING_SEGMENTS;

      sphere_triangles[triangle++] =
          (Triangle){top_left, top_right, bottom_right};
      sphere_triangles[triangle++] =
          (Triangle){top_left, bottom_right, bottom_left};
    }

    sphere_triangles[triangle++] =
        (Triangle){last_ring + segment, last_ring + next_segment, bottom_pole};
  }
}

const char *shapes_name(Shape shape) {
  switch (shape) {
  case SHAPE_TORUS:
    return "Torus";
  case SHAPE_CUBE:
    return "Cube";
  case SHAPE_SPHERE:
    return "Sphere";
  default:
    return "";
  }
}

Mesh shapes_get(Shape shape) {
  switch (shape) {
  case SHAPE_TORUS:
    if (!torus_generated) {
      generate_torus();
      torus_generated = 1;
    }
    return (Mesh){
        .vertices = torus_vertices,
        .triangles = torus_triangles,
        .vertex_count = TORUS_VERTEX_COUNT,
        .triangle_count = TORUS_TRIANGLE_COUNT,
    };
  case SHAPE_CUBE:
    return (Mesh){
        .vertices = cube_vertices,
        .triangles = cube_triangles,
        .vertex_count = 8,
        .triangle_count = 12,
    };
  case SHAPE_SPHERE:
    if (!sphere_generated) {
      generate_sphere();
      sphere_generated = 1;
    }
    return (Mesh){
        .vertices = sphere_vertices,
        .triangles = sphere_triangles,
        .vertex_count = SPHERE_VERTEX_COUNT,
        .triangle_count = SPHERE_TRIANGLE_COUNT,
    };
  default:
    return (Mesh){0};
  }
}
