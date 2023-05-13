#pragma once
#include "vector.h"

struct Intersection {
    Vector3 pos;
    Vector3 geo_normal;
    Vector3 shading_normal;
    Vector2 uv;
    Real t;
    int material_id;
};