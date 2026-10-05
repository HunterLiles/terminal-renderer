#pragma once

#include "renderer/framebuffer.h"
#include "renderer/lighting.h"
#include "sdf.h"

int raymarcher_trace(vec3 origin, vec3 direction, Sdf object, float time,
                     SurfaceSample *surface);
void raymarcher_render(Framebuffer *target, const Lighting *lighting,
                       Sdf object, double time);
