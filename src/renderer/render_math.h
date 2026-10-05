#pragma once

#include <math.h>

typedef struct {
  float x, y;
} vec2;

typedef struct {
  float x, y, z;
} vec3;

static inline vec3 vec3_add(vec3 a, vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline vec3 vec3_sub(vec3 a, vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline vec3 vec3_scale(vec3 v, float scale) {
  return (vec3){v.x * scale, v.y * scale, v.z * scale};
}

static inline float vec3_dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3 vec3_cross(vec3 a, vec3 b) {
  return (vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x};
}

static inline float vec3_length(vec3 v) { return sqrtf(vec3_dot(v, v)); }

static inline vec3 vec3_normalize(vec3 v) {
  float length = vec3_length(v);
  if (length == 0.0f)
    return (vec3){0.0f, 0.0f, 0.0f};
  return (vec3){v.x / length, v.y / length, v.z / length};
}
