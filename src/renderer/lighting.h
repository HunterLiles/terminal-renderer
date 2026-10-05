#pragma once

#include "render_math.h"

typedef struct {
  vec3 position;
  vec3 normal;
  float albedo;
} SurfaceSample;

typedef struct {
  vec3 direction_to_light;
  float intensity;
  float ambient;
} Lighting;

float lighting_shade(const SurfaceSample *surface, const Lighting *lighting);
