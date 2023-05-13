#pragma once
#include "vector.h"

struct Intersection {
    Vector3 pos;
    Vector3 normal;
    Vector2 uv;
    Real t;
    int material_id;
};