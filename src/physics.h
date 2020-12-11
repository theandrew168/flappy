#ifndef FLAPPY_PHYSICS_H_INCLUDED
#define FLAPPY_PHYSICS_H_INCLUDED

#include <stdbool.h>

bool physics_intersect_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh);

#endif
