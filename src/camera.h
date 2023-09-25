#pragma once
#include "vector.h"
#include "parse/parse_scene.h"

struct Camera {
    int width, height;
    Vector3 lookfrom;
    Vector3 lookat;
    Vector3 up;
    Real vfov;
};