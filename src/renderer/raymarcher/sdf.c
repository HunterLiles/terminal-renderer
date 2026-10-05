#include "sdf.h"

static float sphere(vec3 point) { return vec3_length(point) - 1.0f; }

static float sphere_displacement(float time) {
  return 0.28f * (0.5f + 0.5f * cosf(0.8f * time));
}

static float displaced_sphere(vec3 point, float time) {
  float wave_x = sinf(4.0f * point.x + time);
  float wave_y = cosf(4.0f * point.y - 0.7f * time);
  float wave_z = sinf(4.0f * point.z + 0.5f * time);
  float displacement = sphere_displacement(time) * wave_x * wave_y * wave_z;
  return sphere(point) + displacement;
}

static float box(vec3 point) {
  vec3 distance_to_sides = {
      .x = fabsf(point.x) - 0.8f,
      .y = fabsf(point.y) - 0.8f,
      .z = fabsf(point.z) - 0.8f,
  };
  vec3 outside = {
      .x = fmaxf(distance_to_sides.x, 0),
      .y = fmaxf(distance_to_sides.y, 0),
      .z = fmaxf(distance_to_sides.z, 0),
  };
  float nearest_side = fmaxf(distance_to_sides.x, distance_to_sides.y);
  nearest_side = fmaxf(nearest_side, distance_to_sides.z);
  float inside_distance = fminf(nearest_side, 0);
  return vec3_length(outside) + inside_distance;
}

static float torus(vec3 point) {
  const float tilt = 0.6f;
  float local_y = point.y * cosf(tilt) + point.z * sinf(tilt);
  float local_z = -point.y * sinf(tilt) + point.z * cosf(tilt);
  float ring_distance = sqrtf(point.x * point.x + local_z * local_z) - 1.0f;
  float tube_distance =
      sqrtf(ring_distance * ring_distance + local_y * local_y);
  return tube_distance - 0.5f;
}

const char *sdf_name(Sdf object) {
  switch (object) {
  case SDF_DISPLACED_SPHERE:
    return "Displaced sphere";
  case SDF_SPHERE:
    return "Sphere";
  case SDF_BOX:
    return "Box";
  case SDF_TORUS:
    return "Torus";
  default:
    return "";
  }
}

float sdf_step_bound(Sdf object, float time) {
  if (object == SDF_DISPLACED_SPHERE) {
    return 1.0f + sqrtf(3.0f) * sphere_displacement(time) * 4.0f;
  }
  return 1.0f;
}

float sdf_evaluate(Sdf object, vec3 point, float time) {
  vec3 center = {0, 0, 3};
  vec3 local = vec3_sub(point, center);

  if (object == SDF_BOX || object == SDF_TORUS) {
    float rotation_cos = cosf(time);
    float rotation_sin = sinf(time);
    local = (vec3){
        .x = local.x * rotation_cos - local.z * rotation_sin,
        .y = local.y,
        .z = local.x * rotation_sin + local.z * rotation_cos,
    };
  }

  switch (object) {
  case SDF_DISPLACED_SPHERE:
    return displaced_sphere(local, time);
  case SDF_SPHERE:
    return sphere(local);
  case SDF_BOX:
    return box(local);
  case SDF_TORUS:
    return torus(local);
  default:
    return 0;
  }
}
