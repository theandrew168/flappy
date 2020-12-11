#include <math.h>
#include <stdbool.h>

#include "physics.h"

// Based on:
// http://www.jeffreythompson.org/collision-detection/circle-rect.php
//  modified for rx and ry being in the center of the rect
bool
physics_intersect_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh)
{
    float test_x = cx;
    float test_y = cy;
    float half_rw = rw / 2.0f;
    float half_rh = rh / 2.0f;

    if (cx < rx - half_rw) test_x = rx - half_rw;  // left edge
    else if (cx > rx + half_rw) test_x = rx + half_rw;  // right edge
    if (cy < ry - half_rh) test_y = ry - half_rh;  // bottom edge
    else if (cy > ry + half_rh) test_y = ry + half_rh;  // top edge

    // check distance from closest edges
    float dist_x = cx - test_x;
    float dist_y = cy - test_y;
    float distance = sqrtf((dist_x * dist_x) + (dist_y * dist_y));

    return distance <= cr;
}
