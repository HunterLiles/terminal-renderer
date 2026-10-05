#include "lighting.h"

float lighting_shade(const SurfaceSample *surface, const Lighting *lighting) {
  vec3 normal = vec3_normalize(surface->normal);
  vec3 direction = vec3_normalize(lighting->direction_to_light);
  float diffuse = fmaxf(vec3_dot(normal, direction), 0.0f);

  return surface->albedo * (lighting->ambient + lighting->intensity * diffuse);
}
