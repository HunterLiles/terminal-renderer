#include "raymarcher.h"

enum { MAX_STEPS = 256 };
static const float hit_epsilon = 0.001f;
static const float normal_epsilon = 0.001f;
static const float max_distance = 20.0f;

static vec3 surface_normal(vec3 point, Sdf object, float time) {
  vec3 dx = {normal_epsilon, 0, 0};
  vec3 dy = {0, normal_epsilon, 0};
  vec3 dz = {0, 0, normal_epsilon};
  vec3 gradient = {
      sdf_evaluate(object, vec3_add(point, dx), time) -
          sdf_evaluate(object, vec3_sub(point, dx), time),
      sdf_evaluate(object, vec3_add(point, dy), time) -
          sdf_evaluate(object, vec3_sub(point, dy), time),
      sdf_evaluate(object, vec3_add(point, dz), time) -
          sdf_evaluate(object, vec3_sub(point, dz), time),
  };
  return vec3_normalize(gradient);
}

int raymarcher_trace(vec3 origin, vec3 direction, Sdf object, float time,
                     SurfaceSample *surface) {
  direction = vec3_normalize(direction);
  if (vec3_length(direction) == 0.0f)
    return 0;

  float step_bound = sdf_step_bound(object, time);
  float distance = 0.0f;
  for (int step = 0; step < MAX_STEPS && distance <= max_distance; step++) {
    vec3 point = vec3_add(origin, vec3_scale(direction, distance));
    float field = sdf_evaluate(object, point, time);
    if (!isfinite(field))
      return 0;
    if (fabsf(field) < hit_epsilon) {
      *surface = (SurfaceSample){
          .position = point,
          .normal = surface_normal(point, object, time),
          .albedo = 1.0f,
      };
      return 1;
    }
    distance += fabsf(field) / step_bound;
  }
  return 0;
}

void raymarcher_render(Framebuffer *target, const Lighting *lighting,
                       Sdf object, double time) {
  float phase = (float)fmod(time, 62.83185307179586);
  vec3 origin = {0, 0, 0};
  float aspect = (float)target->width / target->height;

  for (int y = 0; y < target->height; y++) {
    for (int x = 0; x < target->width; x++) {
      float u = 2.0f * (x + 0.5f) / target->width - 1.0f;
      float v = 1.0f - 2.0f * (y + 0.5f) / target->height;
      vec3 direction = {u * aspect, v, 1.0f};
      SurfaceSample surface;
      if (raymarcher_trace(origin, direction, object, phase, &surface) &&
          surface.position.z > 0.0f) {
        float brightness = lighting_shade(&surface, lighting);
        framebuffer_write(target, x, y, 1.0f / surface.position.z, brightness);
      }
    }
  }
}
