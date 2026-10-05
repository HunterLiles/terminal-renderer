#pragma once

#include "renderer/render_math.h"

typedef enum {
  SDF_DISPLACED_SPHERE,
  SDF_SPHERE,
  SDF_BOX,
  SDF_TORUS,
  SDF_COUNT,
} Sdf;

const char *sdf_name(Sdf object);
float sdf_evaluate(Sdf object, vec3 point, float time);
float sdf_step_bound(Sdf object, float time);
