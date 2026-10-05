#pragma once

#include "renderer/framebuffer.h"
#include "renderer/lighting.h"
#include "shapes.h"

void rasterizer_render(Framebuffer *target, const Lighting *lighting,
                       Shape shape, double time);
